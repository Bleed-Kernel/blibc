#pragma once

// a double written out for %f %e %g (conv is the letter, its case says the case), no sign.
int floatfmt(char *buf, int cap, double v, char conv, int prec, int alt);
