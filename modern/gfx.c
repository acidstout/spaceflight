/* gfx.c - Implementierung des Indexed-Framebuffers + BGI-Primitiven. */
#include "gfx.h"
#include <string.h>

uint8_t g_fb[FB_H][FB_W];

/* Standard: klassische CGA-Palette 1 (High-Intensity) als RGBA.
   Frei ueberschreibbar -> beliebige TrueColor-Farben. */
rgba_t g_palette[4] = {
    {  0,   0,   0, 255},   /* 0 schwarz  */
    { 85, 255, 255, 255},   /* 1 cyan     */
    {255,  85, 255, 255},   /* 2 magenta  */
    {255, 255, 255, 255},   /* 3 weiss    */
};

static int s_color = 3;
static int s_mx = 0, s_my = 0;   /* MoveTo/LineTo-Position */

/* ---- Font-Zustand ---------------------------------------------------- */
static const uint16_t (*s_font)[16] = 0;   /* 96 Glyphen ab ASCII 32, 16x16 */
static int s_ink_minx = 0, s_ink_maxx = 15, s_ink_miny = 0, s_ink_maxy = 15;
static int s_cellw = 8, s_cellh = 8;
static int s_jh = GFX_LEFT, s_jv = GFX_LEFT;    /* jv: LEFT=oben, CENTER=mittig */

/* Basiszellgroesse fuer die SetUserCharSize-Umrechnung. Die Original-
   Vektorschrift (TriplexFont) ist nicht 1:1 rekonstruierbar; wir bilden die
   Groessenverhaeltnisse auf die skalierte 16x16-Bitmapschrift ab. */
#define FONT_BASE_W 14
#define FONT_BASE_H 20

/* --------------------------------------------------------------------- */

void gfx_clear(int color) {
    memset(g_fb, color & 3, sizeof(g_fb));
}
void gfx_set_color(int c) { s_color = c & 3; }
int  gfx_get_color(void)  { return s_color; }

void gfx_put_pixel(int x, int y, int c) {
    if ((unsigned)x < FB_W && (unsigned)y < FB_H)
        g_fb[y][x] = (uint8_t)(c & 3);
}
int gfx_get_pixel(int x, int y) {
    if ((unsigned)x < FB_W && (unsigned)y < FB_H)
        return g_fb[y][x];
    return 0;
}

void gfx_line(int x1, int y1, int x2, int y2) {
    int dx =  (x2 > x1) ? x2 - x1 : x1 - x2;
    int dy = -((y2 > y1) ? y2 - y1 : y1 - y2);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;
    for (;;) {
        gfx_put_pixel(x1, y1, s_color);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}
void gfx_moveto(int x, int y) { s_mx = x; s_my = y; }
void gfx_lineto(int x, int y) { gfx_line(s_mx, s_my, x, y); s_mx = x; s_my = y; }

void gfx_bar(int x1, int y1, int x2, int y2) {
    int x, y;
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 > FB_W - 1) x2 = FB_W - 1;
    if (y2 > FB_H - 1) y2 = FB_H - 1;
    for (y = y1; y <= y2; y++)
        for (x = x1; x <= x2; x++)
            g_fb[y][x] = 0;   /* SolidFill, Farbe 0 (wie im Original) */
}
void gfx_box(int x1, int y1, int x2, int y2) {
    gfx_moveto(x1, y1);
    gfx_lineto(x2, y1);
    gfx_lineto(x2, y2);
    gfx_lineto(x1, y2);
    gfx_lineto(x1, y1);
}

/* Gemeinsame BGI-Sprite-Ausgabe (2bpp). transparent=1 ueberspringt Farbe 0. */
static void blit(int x, int y, const unsigned char *img, int transparent) {
    int w = (img[0] | (img[1] << 8)) + 1;   /* Header: Breite-1, Hoehe-1 */
    int h = (img[2] | (img[3] << 8)) + 1;
    int stride = (w + 3) / 4;                /* 4 Pixel/Byte, MSB = links */
    const unsigned char *data = img + 4;
    int row, col;
    for (row = 0; row < h; row++) {
        const unsigned char *rp = data + row * stride;
        for (col = 0; col < w; col++) {
            int c = (rp[col >> 2] >> ((3 - (col & 3)) * 2)) & 3;
            if (!transparent || c) gfx_put_pixel(x + col, y + row, c);
        }
    }
}

void gfx_put_image(int x, int y, const unsigned char *img) { blit(x, y, img, 0); }
void gfx_put_image_transp(int x, int y, const unsigned char *img) { blit(x, y, img, 1); }

int gfx_sprite_pixel(const unsigned char *img, int col, int row) {
    int w = (img[0] | (img[1] << 8)) + 1;
    int h = (img[2] | (img[3] << 8)) + 1;
    int stride, b, shift;
    if (col < 0 || row < 0 || col >= w || row >= h) return 0;
    stride = (w + 3) / 4;
    b = img[4 + row * stride + (col >> 2)];
    shift = (3 - (col & 3)) * 2;
    return (b >> shift) & 3;
}

void gfx_get_image(int x1, int y1, int x2, int y2, unsigned char *buf) {
    int w = x2 - x1 + 1, h = y2 - y1 + 1;
    int stride = (w + 3) / 4;
    int row, col;
    buf[0] = (unsigned char)((w - 1) & 0xFF);
    buf[1] = (unsigned char)((w - 1) >> 8);
    buf[2] = (unsigned char)((h - 1) & 0xFF);
    buf[3] = (unsigned char)((h - 1) >> 8);
    for (row = 0; row < h; row++) {
        unsigned char *rp = buf + 4 + row * stride;
        memset(rp, 0, stride);
        for (col = 0; col < w; col++) {
            int c = gfx_get_pixel(x1 + col, y1 + row);
            rp[col >> 2] |= (unsigned char)((c & 3) << ((3 - (col & 3)) * 2));
        }
    }
}

int gfx_sprites_collide(const unsigned char *a, int ax, int ay,
                        const unsigned char *b, int bx, int by) {
    int aw = (a[0] | (a[1] << 8)) + 1, ah = (a[2] | (a[3] << 8)) + 1;
    int bw = (b[0] | (b[1] << 8)) + 1, bh = (b[2] | (b[3] << 8)) + 1;
    int x0 = ax > bx ? ax : bx;
    int y0 = ay > by ? ay : by;
    int x1 = (ax + aw < bx + bw) ? ax + aw : bx + bw;
    int y1 = (ay + ah < by + bh) ? ay + ah : by + bh;
    int px, py;
    for (py = y0; py < y1; py++)
        for (px = x0; px < x1; px++)
            if (gfx_sprite_pixel(a, px - ax, py - ay) &&
                gfx_sprite_pixel(b, px - bx, py - by))
                return 1;
    return 0;
}

/* ---- Text ------------------------------------------------------------ */

void gfx_install_font(const uint16_t (*glyphs)[16],
                      int minx, int maxx, int miny, int maxy) {
    s_font = glyphs;
    if (minx < 0) minx = 0;
    if (maxx > 15) maxx = 15;
    if (miny < 0) miny = 0;
    if (maxy > 15) maxy = 15;
    if (maxx < minx) { minx = 0; maxx = 15; }
    if (maxy < miny) { miny = 0; maxy = 15; }
    s_ink_minx = minx; s_ink_maxx = maxx;
    s_ink_miny = miny; s_ink_maxy = maxy;
}
void gfx_set_justify(int horiz, int vert) { s_jh = horiz; s_jv = vert; }

void gfx_set_char_size(int mulx, int divx, int muly, int divy) {
    if (divx <= 0) divx = 1;
    if (divy <= 0) divy = 1;
    s_cellw = (FONT_BASE_W * mulx + divx / 2) / divx;
    s_cellh = (FONT_BASE_H * muly + divy / 2) / divy;
    if (s_cellw < 3) s_cellw = 3;
    if (s_cellh < 3) s_cellh = 3;
}

static void draw_glyph(int gx, int gy, unsigned char ch, int color) {
    int px, py;
    const uint16_t *g;
    int bw, bh;
    if (!s_font) return;
    if (ch < 32 || ch > 127) ch = 32;
    g = s_font[ch - 32];
    bw = s_ink_maxx - s_ink_minx + 1;   /* getrimmte Ink-Box */
    bh = s_ink_maxy - s_ink_miny + 1;
    /* Coverage-Sampling: ein Zielpixel ist gesetzt, wenn IRGENDEIN Quellpixel
       im abgedeckten Bereich gesetzt ist -> beim Verkleinern gehen keine
       duennen Striche (z.B. Oberkante von e/s) verloren. */
    for (py = 0; py < s_cellh; py++) {
        int sy0 = s_ink_miny + py * bh / s_cellh;
        int sy1 = s_ink_miny + (py + 1) * bh / s_cellh;
        if (sy1 <= sy0) sy1 = sy0 + 1;
        if (sy1 > 16) sy1 = 16;
        for (px = 0; px < s_cellw; px++) {
            int sx0 = s_ink_minx + px * bw / s_cellw;
            int sx1 = s_ink_minx + (px + 1) * bw / s_cellw;
            int yy, on = 0;
            if (sx1 <= sx0) sx1 = sx0 + 1;
            if (sx1 > 16) sx1 = 16;
            for (yy = sy0; yy < sy1 && !on; yy++) {
                uint16_t rowbits = g[yy];
                int xx;
                for (xx = sx0; xx < sx1; xx++)
                    if (rowbits & (0x8000 >> xx)) { on = 1; break; }
            }
            if (on) gfx_put_pixel(gx + px, gy + py, color);
        }
    }
}

static void out_text_at(int x, int y, const char *s, int color) {
    int n = (int)strlen(s);
    int wpx = n * s_cellw;
    int ox = x, oy = y;
    int i;
    if (s_jh == GFX_CENTER) ox = x - wpx / 2;
    else if (s_jh == GFX_RIGHT) ox = x - wpx;
    if (s_jv == GFX_CENTER) oy = y - s_cellh / 2;
    for (i = 0; i < n; i++)
        draw_glyph(ox + i * s_cellw, oy, (unsigned char)s[i], color);
}

void gfx_out_text(int x, int y, const char *s) {
    out_text_at(x, y, s, s_color);
}

/* Repliziert OutTextXY3D aus SPACDATA.PAS: n diagonal versetzte Kopien in
   zyklischen Farben 1..3 (Schatten-/3D-Effekt). */
void gfx_out_text_3d(int x, int y, const char *s, int startcol, int n) {
    int col = startcol, i;
    for (i = 0; i < n; i++) {
        out_text_at(x + i, y + i, s, col);
        col = ((col + 1) % 3) + 1;   /* Succ(Succ(col) mod 3) */
    }
}
