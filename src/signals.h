#ifndef SIGNALS_H
#define SIGNALS_H

/** Sets up the sigactions to ignore the correct signals */
void ignore_signals();

/** Restores modified signals to their default behavior */
void restore_signals();

#endif // SIGNALS_H
