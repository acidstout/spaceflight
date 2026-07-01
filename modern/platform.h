/* platform.h - Fenster, Direct3D 11 Vollbild-Praesentation, Input, Timing,
   Audio und Font-Erzeugung. Ersetzt Crt/Dos + BGI-Bildschirmmodus. */
#ifndef PLATFORM_H
#define PLATFORM_H
#include <stdint.h>

typedef struct {
    int  width;          /* Backbuffer-/Fensterbreite (0 = Desktop-Breite)  */
    int  height;         /* Backbuffer-/Fensterhoehe  (0 = Desktop-Hoehe)   */
    int  fullscreen;     /* 1 = randloses Vollbild                          */
    int  aspect43;       /* 1 = 4:3 (authentisches CRT), 0 = 8:5 quadr. Pixel */
    int  integer_scale;  /* 1 = ganzzahlige, knackige Skalierung (nur 8:5)  */
    int  vsync;          /* 1 = VSync                                        */
    /* Moderne Effekte (0..1): */
    float fx_scanline;
    float fx_glow;
    float fx_vignette;
} plat_config;

int  plat_init(const plat_config *cfg);
void plat_shutdown(void);

/* Nachrichten verarbeiten; liefert 0, wenn das Fenster geschlossen werden soll. */
int  plat_pump(void);

/* g_fb ueber g_palette nach RGBA wandeln, hochskalieren, Effekte, Present. */
void plat_present(void);

/* Zeit in Sekunden (hochaufloesend). */
double plat_time(void);
void   plat_sleep_ms(double ms);

/* --- Tastatur (bildet Crt.Keypressed/ReadKey und die INT16-Funktion Key ab) --- */
int  plat_keypressed(void);            /* Event in der Queue?                 */
int  plat_readkey_char(void);          /* naechstes Event -> ASCII (0 = Ext.) */
int  plat_readkey_scancode(void);      /* naechstes Event -> BIOS-Scancode    */
int  plat_getkey(void);                /* wie GetKey: Zeichen oder 256+Scancode, blockierend */
void plat_clear_input(void);           /* Queue leeren                        */
void plat_inject_key(int sc, int ch);  /* synthetisches Event (Demo/Tests)    */

/* --- Audio (PC-Speaker-Ersatz) --- */
void plat_sound(int freq);
void plat_nosound(void);

/* 16x16-Bitmapfont (ASCII 32..127) zur Laufzeit aus GDI erzeugen. */
const uint16_t (*plat_make_font(void))[16];
/* Ink-Box (belegter Bereich) des erzeugten Fonts - nach plat_make_font gueltig. */
void plat_font_metrics(int *minx, int *maxx, int *miny, int *maxy);

#endif /* PLATFORM_H */
