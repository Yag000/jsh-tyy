# ARCHITECTURE

## Architecture Logicielle

Avant d'entrer dans les détails de l'architecture logicielle, nous trouvons important d'expliquer comment nous nous sommes
organisés vis-à-vis des différentes parties du projet. Nous avons fait le choix tacite d'aborder les propriétés du shell de
manière progressive, en suivant strictement les objectifs de chaque jalon. Cette approche nous a sûrement ralentis quelque
peu, car nous avons dû réécrire certaines parties du code. Cependant, c'est une stratégie qui nous a aidés à éviter de nous
perdre dans le code et qui nous a permis de nous concentrer sur les objectifs de l'étape en cours. De plus, le fait de devoir
réécrire certaines parties du code nous a permis de mieux comprendre les concepts et de les maîtriser.

### Commandes

Le premier choix que nous avons fait concerne les commandes.

Nous avons décidé de séparer autant que possible la logique, afin que chaque partie du code n'accomplisse qu'une seule tâche. Ainsi, la
distinction entre commandes internes et externes est effectuée au dernier moment possible, c'est-à-dire au moment de l'exécution de la commande.

De cette manière, le flux des données est assez simple et suit la séquence suivante :

Lecture de la ligne de commande -> Analyse syntaxique de la ligne de commande -> Exécution de la commande -> Libération des ressources

Qui se traduit dans le code par :

`readline` (`prompt.c`) -> `parse_command` (`command.c`) -> `execute_command_call` (`internals.c`) -> `destroy_command_result` (`command.c`)

Cela ne prend pas en compte la logique derrière les jobs, qui est un peu plus complexe. Nous y reviendrons plus tard. Cependant, toute
commande, qu'elle soit exécutée en arrière-plan ou non, passe par ces fonctions dans cet ordre.

Il peut être agréable de noter qu'une processus lancé par le shell est représenté notamment par un répertoire dans le dossier `/proc` nommé par son propre pid.
Un travail de parsing a été effectué (voir `proc.h` et `proc.c`) afin d'obtenir les enfants d'un processus donné. Cela nous sert particulièrement pour la commande `jobs -t`.

### Jobs

Les jobs sont gérés via une table de jobs. Cette table est initialisée lors de la création du shell et est mise à jour juste avant l'apparition de chaque prompt.
La table de jobs est un tableau redimensionnable de `job` (structure définie dans `jobs.h`), ayant une taille initiale de `JOBS_TABLE_INITIAL_CAPACITY`.
Étant donné que les jobs sont assez peu nombreux, la table est redimensionnée de manière linéaire, en ajoutant à chaque fois `JOBS_TABLE_INITIAL_CAPACITY` cases.
Chaque job est identifié par un ID qui correspond à son index dans la table de jobs plus 1. Ainsi, le premier job a l'ID 1, le deuxième l'ID 2, et ainsi de suite.
Une position vide dans la table est représentée par `NULL`, garantissant que les éléments de la table correspondent à des ressources allouées.

#### Ajout d'un job

Un job est ajouté à la première position vide de la table de jobs. Si la table est pleine, elle est redimensionnée comme nous l'avons mentionné précédemment.
Les jobs sont ajoutés grâce à la fonction `add_job` dans `jobs.h`. Dans notre code, cela se produit dans la fonction `execute_command_call` dans `internals.h`.
Un job est ajouté à la table s'il est exécuté en arrière-plan (`&`) ou s'il est exécuté en premier plan mais suspendu (`CTRL-Z`).

La fonction utilisée est `add_job`(`jobs.c`).

#### Suppression d'un job

Un job est supprimé de la table en écrasant la case du job correspondant dans la table par `NULL`. Cependant, comme la table doit garantir que le job est à la position `id - 1`,
nous ne pouvons pas redimensionner la table à chaque suppression de manière simple.
C'est pour cela que nous avons décidé de ne pas réduire la taille de la table, mais de laisser les cases vides.
Une fois qu'un job arrive à un état dans lequel il est `Done`, `Killed` ou `Detached`, il est supprimé de la table de jobs.
Ceci est testé à chaque mise à jour de la table, c'est-à-dire à chaque fois que le prompt est affiché (ou lorsqu'on exécute `jobs` ou `exit`).

La fonction utilisée est `remove_job`(`jobs.c`).

#### Mise à jour d'un job

À chaque fois que le prompt est affiché, la table de jobs est mise à jour (également lors d'un `jobs` ou d'un `exit`).
Cela se fait en parcourant la table de jobs et en mettant à jour le statut de chaque job.

Pour mettre à jour le statut d'un job, nous parcourons tous les sous-jobs du job et nous appelons la fonction `wait_for_job`
sur chacun d'eux, qui s'occupe d'appeler `waitpid` sur le sous-job s'il n'est pas terminé (c'est-à-dire si une actualisation
passée n'a pas déjà été effectuée et a retourné que le sous-job était terminé).

Pour les mises à jour bloquantes, nous utilisons la fonction `blocking_wait_for_subjob` (`jobs.c`), qui est similaire à
`wait_for_job` (`jobs.c`), mais qui attend que le sous-job soit terminé ou qu'il soit arrêté. Cette fonction est uniquement
appelée après le lancement d'un job en avant-plan ou lors de l'utilisation de `fg`.

Un seul état ne peut pas être déterminé avec l'unique appel à `waitpid`, qui est l'état `DETACHED`. Une fois qu'un job est
fini, on vérifie s'il est `DETACHED` en vérifiant s'il y a encore un processus avec le pgid du job, en utilisant `kill(-pgid, 0)`.

Une fois qu'un job rentre dans un état `DONE`, `KILLED` ou `DETACHED`, il est supprimé de la table de jobs (il est d'abord
affiché s'il est en arrière-plan).

Étapes de la mise à jour d'un job :

On parcourt la table de jobs -> On parcourt les sous-jobs du job -> On met à jour le statut du sous-job -> On met à jour le statut
du job -> On affiche le job si nécessaire -> On supprime le job si nécessaire

Cela se traduit dans le code par dans `jobs.c` :

`update_jobs` -> `wait_for_job` -> `process_waitpid_response` -> `update_job_status` -> `print_job` -> `remove_job` -> `destroy_job`

#### Changement de plan de job

Les fonctions `put_job_in_foreground` et `continue_job_in_background` (`jobs.c`) nous
permettent de changer le plan d'un job afin de pouvoir implémenter les
commandes `fg` et `bg`.

### Gestion des ressources

Une question très importante est de savoir comment bien gérer les ressources, éviter les doubles `free`, fermer les descripteurs de fichiers au bon moment, etc.

Pour les descripteurs de fichiers, la réponse est simple. Avant l'exécution d'une commande, on ferme tous les descripteurs de fichiers qui ne
sont pas utilisés par la commande (les pipes qui ne lui sont pas nécessaires et ceux dont elle dépend mais pas en écriture ou en lecture) entre
le `fork` et l'`execvp`. Une fois la commande lancée (tous les forks pour toutes les sous-commandes ont été réalisés), on ferme tous les descripteurs
de fichiers associés aux commandes (en évitant de fermer `stdin`, `stdout` et `stderr`).

La gestion de la mémoire est principalement assurée par les fonctions `destroy_command_result` et `destroy_job`. Le travail de `destroy_command_result`
est de libérer la mémoire associée aux commandes elles-mêmes une fois qu'elles ont été exécutées. `destroy_job` s'occupe de libérer la mémoire associée
aux jobs une fois qu'ils ont été supprimés de la table de jobs.

### Informations globales du shell

Nous avons une suite de variables globales qui représentent l'état du shell à un moment donné.

- `char lwd[PATH_MAX]` (last working directory) représente le dernier dossier dans lequel on était avant de changer de dossier. Il est initialisé au répertoire courant au lancement du shell.
- `int last_exit_code` représente le code de retour de la dernière commande exécutée, initialisé à `0`.
- `int should_exit` est un entier qui indique si le shell doit s'arrêter ou non. Il est initialisé à `0` et est mis à `1` lorsqu'on exécute la commande `exit` et qu'elle réussit.
- `job **job_table` est un tableau de pointeurs vers des structures `job` qui représente la table de jobs. Il est initialisé au lancement du shell et est redimensionné au besoin.
- `size_t job_table_size` est le nombre de jobs dans la table de jobs. Il est initialisé à `0` et est mis à jour chaque fois qu'un job est ajouté ou supprimé de la table.
- `size_t job_table_capacity` est la capacité de la table de jobs. Elle est initialisée à `INITIAL_JOB_TABLE_CAPACITY` et est redimensionnée au besoin.

Nous avons aussi d'autres variables et macros qui nous servent à définir les spécifications du shell, comme le caractère séparant les commandes, le nombre de commandes internes et leurs noms, etc.

### Signaux

`jsh` doit ignorer les signaux `SIGINT`, `SIGTERM`, `SIGTTIN`, `SIGQUIT`, `SIGTTOU` et `SIGTSTP`. Cela se fait dans notre code grace aux fonctions `ignore_signals` et `restore_signals` dans `signals.c`. Elles sont appelées au lancement du
shell (au premier moment possible) et juste avant le `execvp` lors de l'execution d'une commande respectivement.

## Structure de Données

### Commandes

Pour gérer les commandes, nous utilisons deux structures. La première est la commande proprement dite, qui est une structure de données appelée `command`, dans `command.c`.

- Le nom de la commande.
- Les sous-commandes à exécuter, qui est un `command_call **`. (pour `ls | wc`, il y a deux sous-commandes : `ls` et `wc`).
- Le nombre de sous-commandes.
- Les pipes qui ont été créés pour exécuter la commande. (pour `ls | wc`, il y a un pipe).
- Le nombre de pipes.
- Un entier indiquant si la commande doit être exécutée en arrière-plan ou non.

Pour la gestion des sous-commandes, nous utilisons une structure de données appelée `command_call` qui contient :

- Le nom de la commande.
- Les arguments de la commande.
- Le nombre d'arguments.
- Trois entiers représentant les descripteurs de fichiers pour l'entrée, la sortie et l'erreur standard.
- Deux champs qui indiquent les pipes dont la commande a besoin pour son exécution (en lecture et en écriture). Tout autre pipe présent dans le `command` associé est fermé.
C'est un tableau avec les indices des pipes dans le `command` associé.

La structure `command` est générée par la fonction `parse_command`, prenant en paramètre une ligne de commande, afin d'extraire les informations nécessaires à l'exécution de la commande.

Comme nous l'avons vu précédemment, le `command` généré par `parse_command` est ensuite transmis à la fonction `execute_command` pour son exécution. De cette manière,
la distinction entre les commandes internes et externes, ainsi que celles devant être exécutées en arrière-plan, est réalisée au moment de l'exécution de la commande.
La fonction `parse_command` obtient toutes les informations nécessaires à partir de la ligne de commande, mais c'est `execute_command` qui gère toute la logique
d'exécution, donc aucune distinction n'est faite avant ce moment.

Une fois la commande exécutée (par un appel à `execute_command`), le résultat est stocké dans une structure de données `command_result`, comprenant :

- La commande utilisée (le `command`).
- Le code de retour de la commande. Si la commande est exécutée en arrière-plan, le code de retour a pour valeur `0`.
- Le `pid` et le `job_id` de la commande si elle est exécutée en arrière-plan. Ces champs ont pour valeur `UNINITIALIZED_JOB_ID` et `UNINITIALIZED_PID` si
la commande n'est pas exécutée en arrière-plan.

Ainsi, nous pouvons obtenir le résultat de l'exécution d'une commande en accédant à la structure de données `command_result`
correspondante (retournée par la fonction `execute_command`). Cette structure nous permet non seulement d'obtenir le code de
retour de la commande (qui est également mis à jour dans la variable `last_exit_code`), mais aussi de libérer les ressources
allouées pour la commande au bon moment. De plus, cette structure constitue un moyen rapide de tester le shell dans des tests unitaires et d'intégration.

### Jobs

La gestion des jobs est essentielle pour un shell. Nous avons donc décidé de créer une structure de données appelée `job`, dans `jobs.c`, qui contient :

- L'ID du job (sa position dans la table de jobs + 1).
- Le pgid du job.
- Le statut courant du job (de type `job_status`, qui est une énumération de `RUNNING`, `STOPPED`, `DONE`, `KILLED` et `DETACHED`).
- Le nom de la commande qui a lancé le job.
- Un tableau qui contient les `subjobs` du job (pour `ls | wc`, il y a deux subjobs : `ls` et `wc`).
- Le nombre de subjobs.

Pour les subjobs, nous utilisons une structure de données appelée `subjob` qui contient :

- Le pid du subjob.
- Le statut courant du subjob (de type `subjob_status`).
- Le nom de la commande qui a lancé le subjob.

De cette manière, nous avons accès à toutes les informations nécessaires pour gérer les jobs. Par exemple, pour déterminer si
un job est terminé, nous parcourons tous les subjobs et vérifions s'ils sont tous terminés.

## Algorithmes

### Parsing

Le parsing commence depuis la commande `parse_read_line` qui vient parse l'entrée utilisateur telle quelle. Cette fonction va retourner un array de `command`.  

Le parsing est principalement basé sur l'aide du fichier `string_utils.c` qui nous permet notamment de séparer un `string` en utilisant le séparateur de notre choix, que ce soit ` `, `&` ou `@#[`.
Cette séparation s'effectue grâce à la structure de données `string_iterator` qui contient un string et un `size_t` qui tient compte de la position de notre curseur.

La première étape du parsing est la séparation par `&`, symbole auquel on associe l'exécution d'une commande en arrière-plan. Puis, nous parsons chacun des substrings via `parse_command`.  

Cette dernière fonction est assez simple, elle construit une nouvelle commande vide, nettoie le `string` de la commande en supprimant certains espaces inutiles, le transforme en une sous-commande exécutable, et l'ajoute à la liste des sous-commandes de la commande créée initialement.

Parallèlement, la fonction `parse_command_call` permettant de transformer un `string` en une sous-commande est légèrement plus complexe.
On commence par séparer par ``, puis pour chaque token, on regarde s'il s'agit d'un symbole de redirection, le début d'une substitution ou un pipe.

### Ajout/Suppression d'un job

L'ajout d'un job à la table de jobs se fait en temps linéaire par rapport à la taille de la table. En effet, on parcourt la table
de jobs jusqu'à trouver une case vide ou jusqu'à la fin de la table. Cependant, si la table est pleine, on doit la redimensionner,
ce qui prend un temps linéaire par rapport à la taille de la table.

La suppression d'un job se fait en temps constant, car on ne fait que nettoyer la case correspondant au job dans la table, en appliquant
la fonction `destroy_job` sur le job et en mettant la case à `NULL`.
