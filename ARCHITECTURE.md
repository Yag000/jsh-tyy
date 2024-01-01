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

`readline` -> `parse_command` -> `execute_command_call` -> `destroy_command_result`

Cela ne prend pas en compte la logique derrière les jobs, qui est un peu plus complexe. Nous y reviendrons plus tard. Cependant, toute
commande, qu'elle soit exécutée en arrière-plan ou non, passe par ces fonctions dans cet ordre.

- Prompt : `readline` + `add_to_history` + couleur `$` | `COMMAND_SEPARATOR`
- Var : `last_exit_code` | `lwd[PATH_MAX]` pour cd | `internal_commands[INTERNAL_COMMANDS_COUNT][100]`

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

#### Suppression d'un job

Un job est supprimé de la table en écrasant la case du job correspondant dans la table par `NULL`. Cependant, comme la table doit garantir que le job est à la position `id - 1`,
nous ne pouvons pas redimensionner la table à chaque suppression de manière simple.
C'est pour cela que nous avons décidé de ne pas réduire la taille de la table, mais de laisser les cases vides.
Une fois qu'un job arrive à un état dans lequel il est `Done`, `Killed` ou `Detached`, il est supprimé de la table de jobs.
Ceci est testé à chaque mise à jour de la table, c'est-à-dire à chaque fois que le prompt est affiché (ou lorsqu'on exécute `jobs` ou `exit`).

### Gestion des ressources

Une question très importante est de savoir comment bien gérer les ressources, éviter les double `free`, fermer les descripteurs de fichiers au bon moment, etc.
Ceci est géré de manière très simple : l'appel à `destroy_job`, `destroy_command_result` ou `destroy_command_call` détruit toutes les ressources allouées par la structure,
y compris les ressources allouées par les structures contenues dans la structure.
Ainsi, en appelant `destroy_job` sur un job, on parvient à supprimer tout et à fermer tous les descripteurs de fichiers.
Le fait de ne pas libérer deux fois la même ressource est assuré par le fait que la fonction `destroy_job` est appelée à un seul endroit dans le code (de même pour la fonction
`destroy_command_result` pour les commandes internes et externes en premier plan),
et que la case contenant l'information du job (la seule référence à cette information) est mise à `NULL` après l'appel à `destroy_job`.

Un seul problème pourrait survenir : lors du lancement d'un processus en arrière-plan, un `command_status` est alloué, ce qui pourrait poser des problèmes s'il contenait une référence vers
le même `command_call` que le job. Cependant, ce problème est évité en mettant à `NULL` le `command_call` du `command_status` avant de le mettre
dans le job. Ainsi, on assure que le `command_call` ne sera pas libéré deux fois.

## Structure de Données

TODO: Petite intro sur les structures de données

### Commandes

Pour simplifier la gestion des commandes, nous avons choisi de les stocker dans une structure de données appelée `command_call`. Cette structure contient les éléments suivants :

- Le nom de la commande.
- Les arguments de la commande.
- Le nombre d'arguments.
- Un entier indiquant si la commande doit être exécutée en arrière-plan ou non.
- Trois entiers représentant les descripteurs de fichiers pour l'entrée, la sortie et l'erreur standard.

Cette structure est générée par la fonction `parse_command`, prenant en paramètre une ligne de commande, afin d'extraire les informations nécessaires à l'exécution de la commande.

Comme nous l'avous vu précédemment, le `command_call` généré par `parse_command` est ensuite transmis à la fonction `execute_command_call` pour son exécution.
De cette manière, la distinction entre les commandes internes et externes, ainsi que celles devant être exécutées en arrière-plan, est réalisée au moment de l'exécution
de la commande. La fonction `parse_command` obtient toutes les informations nécessaires à partir de la ligne de commande, mais c'est `execute_command_call` qui gère toute la logique
d'exécution, donc aucune distinction n'est faite avant ce moment.

Une fois la commande exécutée (par un appel à `execute_command_call`), le résultat est stocké dans une structure de données `command_result`, comprenant :

- La commande utilisée (le `command_call`). Ce champ a pour valeur `NULL` si la commande est exécutée en arrière-plan, car elle sera alors stockée dans la `job_table`.
- Le code de retour de la commande. Si la commande est exécutée en arrière-plan, le code de retour a pour valeur `0`.
- Le `pid` et le `job_id` de la commande si elle est exécutée en arrière-plan. Ces champs ont pour valeur `UNINITIALIZED_JOB_ID` et `UNINITIALIZED_PID`, si la commande n'est pas exécutée en arrière-plan.

Ainsi, nous pouvons obtenir le résultat de l'exécution d'une commande en accédant à la structure de données `command_result` correspondante (retournée par la fonction `execute_command_call`).
Cette structure nous permet non seulement d'obtenir le code de retour de la commande (qui est également mis à jour dans la variable `last_exit_code`), mais aussi de libérer les ressources allouées
pour la commande (les descripteurs de fichiers et les arguments) au bon moment. De plus, cette structure constitue un moyen rapide de tester le shell dans des tests unitaires et d'intégration.

### Jobs

TODO

## Algorithmes

- String Parsing | & Parsing

### Ajout/Suppression d'un job

L'ajout d'un job à la table de jobs se fait en temps linéaire par rapport à la taille de la table. En effet, on parcourt la table
de jobs jusqu'à trouver une case vide ou jusqu'à la fin de la table. Cependant, si la table est pleine, on doit la redimensionner,
ce qui prend un temps linéaire par rapport à la taille de la table.

La suppression d'un job se fait en temps constant, car on ne fait que nettoyer la case correspondant au job dans la table, en appliquant
la fonction `destroy_job` sur le job et en mettant la case à `NULL`.
