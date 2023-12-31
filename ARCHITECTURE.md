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
- `job_table`

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
- Ajout des jobs à la job_table + suppression
