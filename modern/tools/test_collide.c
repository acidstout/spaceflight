/* Verifiziert gfx_sprites_collide: Box-Ueberlappung vs. Pixel-Ueberlappung. */
#include "../gfx.h"
#include "../sprites.h"
#include <stdio.h>

static int rects_overlap(const unsigned char *a, int ax, int ay,
                         const unsigned char *b, int bx, int by) {
    int aw = (a[0]|(a[1]<<8))+1, ah = (a[2]|(a[3]<<8))+1;
    int bw = (b[0]|(b[1]<<8))+1, bh = (b[2]|(b[3]<<8))+1;
    return ax < bx+bw && ax+aw > bx && ay < by+bh && ay+ah > by;
}

int main(void) {
    int bx, by;
    long box_only = 0, pixel = 0, both_far = 0, invariant_ok = 1;
    /* Jet fest bei (0,0), Meteorit rundherum verschieben. */
    for (by = -30; by <= 35; by++)
        for (bx = -30; bx <= 63; bx++) {
            int box = rects_overlap(Jet, 0, 0, Meteorit, bx, by);
            int pix = gfx_sprites_collide(Jet, 0, 0, Meteorit, bx, by);
            if (pix && !box) invariant_ok = 0;      /* Pixel => Box muss gelten */
            if (box && !pix) box_only++;
            if (pix) pixel++;
        }
    /* Weit entfernt: keine Kollision. */
    both_far = gfx_sprites_collide(Jet, 0, 0, Meteorit, 200, 200);

    printf("Pixel-Treffer gesamt      : %ld\n", pixel);
    printf("Nur-Box (Box ja, Pixel nein): %ld  <- genau die Faelle, die die Option abschaltet\n", box_only);
    printf("Weit entfernt (muss 0)    : %ld\n", both_far);
    printf("Invariante Pixel=>Box     : %s\n", invariant_ok ? "OK" : "VERLETZT");

    if (pixel > 0 && box_only > 0 && both_far == 0 && invariant_ok) {
        printf("ERGEBNIS: OK - Pixelkollision unterscheidet sich korrekt von der Box.\n");
        return 0;
    }
    printf("ERGEBNIS: FEHLER\n");
    return 1;
}
