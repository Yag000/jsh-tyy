# ARCHITECTURE

## Architecture Logicielle

- Commandes internes / externes -> quasiment indissociables jusqu'à l'exécution.
- Prompt : `readline` + `add_to_history` + couleur `$` | `COMMAND_SEPARATOR`
- Var : `last_exit_code` | `lwd[PATH_MAX]` pour cd | `internal_commands[INTERNAL_COMMANDS_COUNT][100]`
- `job_table`

## Structure de Données

- `command-call`
- `command-result`
- `job`

## Algorithmes

- String Parsing | & Parsing
- Ajout des jobs à la job_table + suppression
