#ifndef DARRAY_H
#define DARRAY_H

#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

#define ADD_ELEMENTS(SIZE, CAP, PTR, N) ((void)((SIZE + (N)) > CAP && (CAP = MAX(CAP * 2, SIZE + (N)), PTR = realloc(PTR, sizeof *PTR * CAP))), SIZE += (N), PTR + SIZE - (N))
#define ADD_ELEMENT(SIZE, CAP, PTR) *((void)(SIZE >= CAP ? (CAP = MAX(CAP * 2, 1)) : 0, PTR = realloc(PTR, sizeof *PTR * CAP)), PTR + SIZE++)

#endif
