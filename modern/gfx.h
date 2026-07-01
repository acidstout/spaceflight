/* gfx.h - 320x200 Indexed-Framebuffer + BGI-kompatible Primitiven.
   Ersetzt die Turbo-Pascal Graph-Unit. Alle Spiel-Koordinaten bleiben im
   Original-320x200-Raum; das Hochskalieren auf die echte Aufloesung macht
   die Plattform-Schicht (platform_win.c) beim Praesentieren. */
#ifndef GFX_H
#define GFX_H
#include <stdint.h>

#define FB_W 320
#define FB_H 200

/* Logischer Framebuffer: ein Byte pro Pixel = Palettenindex 0..3.
   Die gesamte Original-Kollisions- und Sternenlogik (GetPixel) arbeitet
   unveraendert gegen diesen Puffer. */
extern uint8_t g_fb[FB_H][FB_W];

typedef struct { uint8_t r, g, b, a; } rgba_t;
extern rgba_t g_palette[4];   /* frei einstellbar -> echtes TrueColor/RGBA */

/* Justify-Konstanten wie in BGI (LeftText/CenterText/RightText). */
enum { GFX_LEFT = 0, GFX_CENTER = 1, GFX_RIGHT = 2 };

void gfx_clear(int color);
void gfx_set_color(int c);
int  gfx_get_color(void);

void gfx_put_pixel(int x, int y, int c);   /* geclippt auf 0..319 / 0..199 */
int  gfx_get_pixel(int x, int y);          /* out-of-range -> 0 (schwarz) */

void gfx_line(int x1, int y1, int x2, int y2);   /* aktuelle Zeichenfarbe */
void gfx_moveto(int x, int y);
void gfx_lineto(int x, int y);
void gfx_bar(int x1, int y1, int x2, int y2);    /* Fuellrechteck, Farbe 0 (wie im Spiel) */
void gfx_box(int x1, int y1, int x2, int y2);    /* Rahmen in aktueller Farbe */

/* CopyPut: kopiert das komplette Sprite inkl. Farbe 0 (so loescht das Spiel
   Objekte, indem es "Loesch"-Sprites zeichnet). Fuer HUD/Menue/Titel. */
void gfx_put_image(int x, int y, const unsigned char *img);

/* Transparente Variante: Farbe 0 wird uebersprungen -> Objekte ueberlappen
   korrekt (kein schwarzer Kasten). Fuer die Spielobjekte im Gameplay. */
void gfx_put_image_transp(int x, int y, const unsigned char *img);

/* Farbindex eines Sprite-Pixels (0 = leer/schwarz), 0 ausserhalb. */
int gfx_sprite_pixel(const unsigned char *img, int col, int row);

/* Rechteck aus dem Framebuffer ins BGI-Format kodieren (Gegenstueck zu
   PutImage / Turbo-Pascal GetImage). buf muss gross genug sein:
   4 + ((x2-x1+4)/4) * (y2-y1+1) Bytes. */
void gfx_get_image(int x1, int y1, int x2, int y2, unsigned char *buf);

/* Pixel-genaue Kollision zweier Sprites (BGI-Format) an Position (ax,ay)
   bzw. (bx,by): 1, wenn es eine Stelle gibt, an der BEIDE Sprites ein
   nicht-schwarzes Pixel (Farbe != 0) haben. */
int gfx_sprites_collide(const unsigned char *a, int ax, int ay,
                        const unsigned char *b, int bx, int by);

/* Text. Font wird von der Plattform zur Laufzeit aus GDI erzeugt
   (96 Glyphen, ASCII 32..127, je 16x16 Bit -> sauber skalierbar).
   Die Ink-Box (minx..maxx / miny..maxy) trimmt leere Raender, damit die
   Buchstaben die Zelle fuellen. */
void gfx_install_font(const uint16_t (*glyphs)[16],
                      int minx, int maxx, int miny, int maxy);
void gfx_set_char_size(int mulx, int divx, int muly, int divy);
void gfx_set_justify(int horiz, int vert);
void gfx_out_text(int x, int y, const char *s);
void gfx_out_text_3d(int x, int y, const char *s, int startcol, int n);

#endif /* GFX_H */
