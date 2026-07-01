/* game.c - Portierung von SPACE.PAS + SPACDATA.PAS (Implementierungsteil)
   nach C. Die Struktur und Prozedurnamen folgen bewusst dem Original, damit
   sich der 1989er-Turbo-Pascal-Code 1:1 nachvollziehen laesst.

   Grafik/Timing/Input/Audio laufen ueber gfx.* (320x200-Indexpuffer) und
   platform.* (Direct3D-11-Vollbild, Upscale-Effektshader). */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "gfx.h"
#include "platform.h"
#include "sprites.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ---------------------------------------------------------------------- */
/* Konstanten (aus SPACDATA.PAS)                                          */
/* ---------------------------------------------------------------------- */
#define MaxStar             30
#define MaxMeteoritStar     40
#define MaxPageStar        110
#define MaxJetStar          35
#define MaxFuelCount        30
#define MaxSpeed           100
#define MinSpeed             4
#define MinY                21
#define MaxY               172
#define MinX                 4
#define MaxX               240
#define ShootAdd             8
#define EnemyShootAdd        6
#define MeteorRandom       100
#define UfoRandomAnf       200
#define BarrelRandom       956
#define WuerfelRandomAnf  1857
#define BrockenRandomAnf   241
#define EnemyRandomAnf     109
#define EnemyShootRandomAnf 35
#define MaxWuerfelCount     32
#define MaxUfoAnz            5
#define MaxBrockenAnzAnf     5
#define MaxMaxBrockenAnz    20
#define MaxShootAnz          7
#define SchwierigerRandom   11

/* ---------------------------------------------------------------------- */
/* Kompatibilitaets-Wrapper: bilden die Turbo-Pascal-/BGI-Aufrufe ab.     */
/* ---------------------------------------------------------------------- */
#define PutImage(px,py,img)  gfx_put_image((px),(py),(const unsigned char*)(img))
/* Transparent (Farbe 0 = durchsichtig) fuer Spielobjekte -> korrekte
   Ueberlappung statt schwarzem Kasten. */
#define PutImageT(px,py,img) gfx_put_image_transp((px),(py),(const unsigned char*)(img))
#define GetImage(x1,y1,x2,y2,buf) gfx_get_image((x1),(y1),(x2),(y2),(buf))
#define PutPixel             gfx_put_pixel
#define GetPixel             gfx_get_pixel
#define SetColor             gfx_set_color
#define Line                 gfx_line
#define Bar                  gfx_bar
#define Box                  gfx_box
#define MoveTo               gfx_moveto
#define LineTo               gfx_lineto
#define OutTextXY            gfx_out_text
#define OutTextXY3D          gfx_out_text_3d
#define SetTextJustify       gfx_set_justify
#define SetUserCharSize      gfx_set_char_size

#define Random(n)            rnd(n)
#define Succ(x)              ((x)+1)
#define Pred(x)              ((x)-1)

#define Keypressed()         plat_keypressed()
#define Key()                plat_readkey_scancode()
#define ClearInput()         plat_clear_input()
/* ReadKey/GetKey praesentieren erst den aktuellen Frame, dann blockieren. */
#define ReadKey()            (plat_present(), plat_readkey_char())
#define GetKey()             (plat_present(), plat_getkey())

static int rnd(int n) { return n > 0 ? (rand() % n) : 0; }

static void Sound(int f) { plat_sound(f); }
static void NoSound(void) { plat_nosound(); }

/* SetScreenMode/SetGraphMode loeschten in BGI den Bildschirm auf Schwarz. */
static void SetScreenMode(void) { gfx_clear(0); }

/* ---------------------------------------------------------------------- */
/* Globale Zustandsvariablen (aus SPACDATA.PAS VAR)                       */
/* ---------------------------------------------------------------------- */
static int mx, my, sx, sy, ex, ey, bx, by, bmove, Fuel;
static int esx, esy1, esy2, exadd, wx, wy, wxmove, ShootCount;
static int exmove, eymove, ShipMoveY, ShipMoveX;
static int UfoRandom, WuerfelRandom, BrockenRandom, EnemyRandom, EnemyShootRandom;
static int MaxBrockenAnz, Taste, StarMaxSpeed, MeteorDestroyedCount;
static int p, WuerfelCount, BrockenCount, BrockenAnz, SpeedDelay;
static int FuelCount, UfoCount;
static int x, y, i, j;
static int Lives, mxadd, Ufonr;
static char a;
static char Nameein[16];
static long Score;

static int Meteor, MeteorDestroyed, Finished, BarrelOnScreen;
static int EnemyOnScreen, BrockenOnScreen, WuerfelOnScreen, SoundOn;
static int EnemyShootOnScreen, Shoot;
static int PixelCollision;   /* 0 = Original-Rechteck, 1 = pixelgenau */

static int OldStarX[MaxStar+1], StarX[MaxStar+1], StarY[MaxStar+1];
static int StarMove[MaxStar+1];
static int MStarX[MaxMeteoritStar+1], MStarY[MaxMeteoritStar+1];
static int MStarMoveX[MaxMeteoritStar+1], MStarMoveY[MaxMeteoritStar+1];
static int OldMStarX[MaxMeteoritStar+1], OldMStarY[MaxMeteoritStar+1];
static int SStarX[MaxPageStar+1], SStarY[MaxPageStar+1];
static int SStarMoveX[MaxPageStar+1], SStarMoveY[MaxPageStar+1];
static int OldSStarX[MaxPageStar+1], OldSStarY[MaxPageStar+1];
static int JStarX[MaxJetStar+1], JStarY[MaxJetStar+1];
static int JStarMoveX[MaxJetStar+1], JStarMoveY[MaxJetStar+1];
static int OldJStarX[MaxJetStar+1], OldJStarY[MaxJetStar+1];
static int ShootX[MaxShootAnz+1], ShootY[MaxShootAnz+1];
static int ux[MaxUfoAnz+1], uy[MaxUfoAnz+1], uxadd[MaxUfoAnz+1], uyadd[MaxUfoAnz+1];
static int BrockenX[MaxMaxBrockenAnz+1], BrockenY[MaxMaxBrockenAnz+1];
static int BrockenmoveX[MaxMaxBrockenAnz+1];
static int BrockenNr[MaxMaxBrockenAnz+1];
static int Brockenweg[MaxMaxBrockenAnz+1];
static int UfoOnScreen[MaxUfoAnz+1];

/* Highscores als Ganzzahlen (eigenes, sauberes Dateiformat). */
static long HiVal[9];
static char HiName[9][16];

/* Vollbild-Sicherungspuffer (fuer Pause / Titelanimation), BGI-Format. */
static unsigned char Puffer[16400];

/* ---------------------------------------------------------------------- */
/* Frame-Steuerung / Timing / Quit                                        */
/* ---------------------------------------------------------------------- */
static int g_running = 1;
static double g_last_frame;
static int g_base_fps = 60;

/* Aktive Anzeige-Konfiguration (wird im Setup live geaendert). */
static plat_config g_cfg;
static const int g_res_presets[6][2] = {
    {1280, 720}, {1366, 768}, {1600, 900},
    {1920, 1080}, {2560, 1440}, {3840, 2160}
};
#define N_RES 6
static int g_res_index = 3;   /* Standard-Fenstergroesse 1920x1080 */

/* ---- Demo-/Selbsttest-Modus ------------------------------------------ */
static int g_demo = 0;
static int g_scene = 0;          /* 1=Titel 2=Menue 3=Spiel 4=Setup 5=Anleitung */
static int g_scene_prev = -1;
static int g_scene_frame = 0;

static void dump_bmp(const char *path) {
    FILE *f = fopen(path, "wb");
    int W = FB_W, H = FB_H, xx, yy;
    int rowsz = (W * 3 + 3) & ~3, datasz = rowsz * H;
    unsigned char hdr[54] = {0};
    if (!f) return;
    hdr[0]='B'; hdr[1]='M';
    *(int*)(hdr+2)=54+datasz; *(int*)(hdr+10)=54; *(int*)(hdr+14)=40;
    *(int*)(hdr+18)=W; *(int*)(hdr+22)=H;
    *(short*)(hdr+26)=1; *(short*)(hdr+28)=24; *(int*)(hdr+34)=datasz;
    fwrite(hdr,1,54,f);
    for (yy = H-1; yy >= 0; yy--) {
        unsigned char line[FB_W*3+4] = {0};
        for (xx = 0; xx < W; xx++) {
            rgba_t c = g_palette[g_fb[yy][xx] & 3];
            line[xx*3+0]=c.b; line[xx*3+1]=c.g; line[xx*3+2]=c.r;
        }
        fwrite(line,1,rowsz,f);
    }
    fclose(f);
}

static void demo_tick(void) {
    int f;
    if (g_scene != g_scene_prev) { g_scene_prev = g_scene; g_scene_frame = 0; }
    f = ++g_scene_frame;
    if (g_scene == 1) {                    /* Titel */
        if (f == 40) dump_bmp("shot_title.bmp");
        if (f == 50) plat_inject_key(57, ' ');
    } else if (g_scene == 2) {             /* Menue */
        if (f == 25) dump_bmp("shot_menu.bmp");
        if (f == 35) plat_inject_key(0, (g_demo == 2) ? '4' : (g_demo == 3) ? '3' : '2');
    } else if (g_scene == 5) {             /* Anleitung (nur --demoexplain) */
        if (f >= 12 && (f - 12) % 22 == 0 && f <= 12 + 22 * 6) {
            char nm[24];
            int page = (f - 12) / 22 + 1;
            sprintf(nm, "shot_ex%d.bmp", page);
            dump_bmp(nm);
            if (page < 7) plat_inject_key(57, ' ');   /* naechste Seite */
            else g_running = 0;                        /* nach Seite 7 stoppen */
        }
    } else if (g_scene == 4) {             /* Setup (nur --demosetup) */
        if (f == 25) dump_bmp("shot_setup.bmp");
        if (f == 35) {                              /* Speed-Feld: "42" eintippen */
            plat_inject_key(0, '2');
            plat_inject_key(0, '4');
            plat_inject_key(0, '2');
            plat_inject_key(28, 13);
        }
        if (f == 60) dump_bmp("shot_setup2.bmp");
        if (f == 75) g_running = 0;
    } else if (g_scene == 3) {             /* Spiel */
        if (f == 15)  plat_inject_key(80, 0);      /* runter */
        if (f == 40)  plat_inject_key(57, ' ');    /* Schuss */
        if (f == 55)  plat_inject_key(57, ' ');
        if (f == 75)  plat_inject_key(72, 0);      /* hoch   */
        if (f == 95)  plat_inject_key(57, ' ');
        if (f == 115) plat_inject_key(77, 0);      /* rechts */
        if (f == 150) dump_bmp("shot_game.bmp");
        if (f == 230) { dump_bmp("shot_game2.bmp"); g_running = 0; }
    }
}

static void frame(void) {
    double now, target, dt;
    if (g_demo) demo_tick();
    plat_present();
    if (!plat_pump()) g_running = 0;
    now = plat_time();
    target = 1.0 / (double)g_base_fps + (double)SpeedDelay * 0.001;
    dt = now - g_last_frame;
    if (dt < target) plat_sleep_ms((target - dt) * 1000.0);
    g_last_frame = plat_time();
}

static void install_font(void) {
    const uint16_t (*fnt)[16] = plat_make_font();
    int a1, a2, a3, a4;
    plat_font_metrics(&a1, &a2, &a3, &a4);
    gfx_install_font(fnt, a1, a2, a3, a4);
}

/* Anzeige (Fenster/Vollbild + Aufloesung) zur Laufzeit neu aufsetzen. */
static void apply_display(void) {
    plat_shutdown();
    plat_init(&g_cfg);
    install_font();
    g_last_frame = plat_time();
}

/* Val fuer ein einzelnes Ziffernzeichen (wie Val(Char,Byte,Check)). */
static int ValChar(char c, int *out) {
    if (c >= '0' && c <= '9') { *out = c - '0'; return 0; }
    return 1;
}

/* ---------------------------------------------------------------------- */
/* Seiten-Sterne (Menue-/Info-Hintergrund) - aus SPACDATA.PAS            */
/* ---------------------------------------------------------------------- */
static void MovePageStars(void) {
    int i;
    for (i = 1; i <= MaxPageStar; i++) {
        PutPixel(OldSStarX[i] >> 2, OldSStarY[i] >> 2, 0);
        if (GetPixel(SStarX[i] >> 2, SStarY[i] >> 2) == 0) {
            PutPixel(SStarX[i] >> 2, SStarY[i] >> 2, 3);
            OldSStarX[i] = SStarX[i];
            OldSStarY[i] = SStarY[i];
        } else OldSStarX[i] = 1290;
        SStarX[i] += SStarMoveX[i];
        SStarY[i] += SStarMoveY[i];
        if (SStarX[i] < 4 || SStarX[i] > 1280 || SStarY[i] < 4 || SStarY[i] > 800) {
            PutPixel(OldSStarX[i] >> 2, OldSStarY[i] >> 2, 0);
            SStarX[i] = (sx + 12) << 2;
            SStarY[i] = (sy + 12) << 2;
            SStarMoveX[i] = (Random(45) - 22) * 2;
            SStarMoveY[i] = (Random(45) - 22) * 2;
            if (SStarMoveX[i] == 0) SStarMoveX[i] = 9;
        }
    }
}

static void InitPageStars(void) {
    int i;
    for (i = 1; i <= MaxPageStar; i++) {
        SStarX[i] = sx << 2;
        SStarY[i] = sy << 2;
        SStarMoveX[i] = Random(45) - 22;
        SStarMoveY[i] = Random(45) - 22;
        OldSStarX[i] = 1290;
    }
}

/* Wartet mit Sternen-Animation, bis eine Taste kommt; konsumiert sie. */
static void wait_stars(void) {
    while (g_running && !Keypressed()) { MovePageStars(); frame(); }
    if (g_running) a = (char)ReadKey();   /* nicht blockieren, wenn Quit angefordert */
}

/* ---------------------------------------------------------------------- */
/* Highscore-Datei                                                        */
/* ---------------------------------------------------------------------- */
static void LoadHighScores(void) {
    FILE *f = fopen("SPACE.HSC", "r");
    int ok = 0, k;
    if (f) {
        ok = 1;
        for (k = 1; k <= 8; k++) {
            char line[64];
            if (!fgets(line, sizeof(line), f)) { ok = 0; break; }
            /* Format: "<wert>|<name>" */
            char *bar = strchr(line, '|');
            if (!bar) { ok = 0; break; }
            *bar = 0;
            HiVal[k] = atol(line);
            char *nm = bar + 1;
            nm[strcspn(nm, "\r\n")] = 0;
            snprintf(HiName[k], sizeof HiName[k], "%s", nm);
        }
        if (ok) {
            char line[64];
            PixelCollision = 0;
            if (fgets(line, sizeof(line), f)) SpeedDelay = atoi(line);
            if (fgets(line, sizeof(line), f)) SoundOn = (line[0] != '0');
            if (fgets(line, sizeof(line), f)) PixelCollision = (line[0] != '0');
            /* Anzeige: Vollbild-Flag, Breite, Hoehe (optional). */
            if (fgets(line, sizeof(line), f)) g_cfg.fullscreen = (line[0] != '0');
            if (fgets(line, sizeof(line), f)) g_cfg.width = atoi(line);
            if (fgets(line, sizeof(line), f)) g_cfg.height = atoi(line);
        }
        fclose(f);
    }
    if (!ok) {
        for (k = 1; k <= 8; k++) {
            HiVal[k] = (long)(9 - k) * 3000;
            strcpy(HiName[k], "--- ABCS ---");
        }
        SpeedDelay = 0;
        SoundOn = 0;
        PixelCollision = 0;
    }
    /* g_res_index an gespeicherte Fenstergroesse angleichen. */
    for (k = 0; k < N_RES; k++)
        if (g_res_presets[k][0] == g_cfg.width && g_res_presets[k][1] == g_cfg.height)
            g_res_index = k;
}

static void SaveHighScores(void) {
    FILE *f = fopen("SPACE.HSC", "w");
    int k;
    if (!f) return;
    for (k = 1; k <= 8; k++)
        fprintf(f, "%ld|%s\n", HiVal[k], HiName[k]);
    fprintf(f, "%d\n", SpeedDelay);
    fprintf(f, "%d\n", SoundOn ? 1 : 0);
    fprintf(f, "%d\n", PixelCollision ? 1 : 0);
    fprintf(f, "%d\n", g_cfg.fullscreen ? 1 : 0);
    fprintf(f, "%d\n", g_cfg.width);
    fprintf(f, "%d\n", g_cfg.height);
    fclose(f);
}

/* ---------------------------------------------------------------------- */
/* Grafische Texteingabe (GraphRead aus SPACDATA.PAS)                     */
/* ---------------------------------------------------------------------- */
static void GraphRead(int gx, int gy, char *out) {
    /* Zeichen und Cursor sitzen buendig auf der Zeile [gy, gy+CH]. CW/CH
       passen zu SetUserCharSize(12,16,12,16); +2 deckt den 3D-Schatten ab. */
    const int CW = 11, CH = 15;
    int xpos = gx, len = 0;
    out[0] = 0;
    SetUserCharSize(12, 16, 12, 16);
    SetTextJustify(GFX_LEFT, GFX_LEFT);
    for (;;) {
        int z;
        SetColor(3);
        Line(xpos, gy, xpos, gy + CH);            /* Cursor auf Texthoehe */
        z = GetKey();
        if (z == 8) {                             /* Backspace */
            if (len > 0) {
                len--; out[len] = 0;
                Bar(xpos, gy, xpos + CW + 2, gy + CH + 2);   /* Cursor weg */
                xpos -= CW;
                Bar(xpos, gy, xpos + CW + 2, gy + CH + 2);   /* letztes Zeichen weg */
            }
            continue;
        }
        if (z == 13) break;                       /* Enter */
        if (z > 31 && z < 256 && len < 12) {
            char s[2]; s[0] = (char)z; s[1] = 0;
            Bar(xpos, gy, xpos + CW + 2, gy + CH + 2);
            OutTextXY3D(xpos, gy, s, 1, 3);
            xpos += CW;
            out[len++] = (char)z; out[len] = 0;
        }
    }
    Bar(xpos, gy, xpos + CW + 2, gy + CH + 2);     /* Cursor entfernen */
}

/* ---------------------------------------------------------------------- */
/* HUD / Anzeige (aus SPACE.PAS)                                          */
/* ---------------------------------------------------------------------- */
static void PrintLives(void) { PutImage(180 + Lives * 19, 5, Leben); }
static void DelLive(void)     { PutImage(180 + Succ(Lives) * 19, 5, LebenLoesch); }

static void PrintScore(void) {
    char hstr[16];
    Bar(50, 3, 112, 11);
    SetColor(3);
    sprintf(hstr, "%7ld", Score);
    OutTextXY(50, 3, hstr);
    if (Random(SchwierigerRandom) == 0) {
        WuerfelRandom += Random(47);
        if (WuerfelRandom > 10000) WuerfelRandom = 10000;
        BrockenRandom -= Random(15);
        if (BrockenRandom < 39) BrockenRandom = 39;
        EnemyRandom -= Random(7);
        if (EnemyRandom < 31) EnemyRandom = 31;
        EnemyShootRandom -= Random(4);
        if (EnemyShootRandom < 7) EnemyShootRandom = 7;
        if (Random(16) == 0) MaxBrockenAnz = Succ(MaxBrockenAnz);
        if (MaxBrockenAnz > MaxMaxBrockenAnz) MaxBrockenAnz = MaxMaxBrockenAnz;
    }
}

static void PrintFuel(void) {
    SetColor(0);
    Line(Fuel + 4, 13, Fuel + 4, 15);
}

static void draw_fuel_bar_full(void) {
    SetColor(2);
    Line(4, 13, 315, 13); Line(4, 14, 315, 14); Line(4, 15, 315, 15);
    SetColor(3);
    Line(4, 13, 50, 13); Line(4, 14, 50, 14); Line(4, 15, 50, 15);
}

static void InitScreen(void) {
    SetScreenMode();
    SetColor(1);
    MoveTo(0, 0); LineTo(319, 0); LineTo(319, 17); LineTo(0, 17); LineTo(0, 0);
    draw_fuel_bar_full();
    SetColor(2);
    SetUserCharSize(8, 16, 8, 16);
    SetTextJustify(GFX_LEFT, GFX_LEFT);
    OutTextXY(3, 3, "Score:");
    PrintScore();
    OutTextXY(140, 3, "Lives:");
    for (Lives = 1; Lives <= 6; Lives++) PrintLives();
    Lives = 6;
}

/* ---------------------------------------------------------------------- */
/* Init des Spielzustands (InitGame aus SPACE.PAS)                        */
/* ---------------------------------------------------------------------- */
static void InitGame(void) {
    for (i = 1; i <= MaxStar; i++) {
        StarX[i] = Random(2560);
        OldStarX[i] = 2560;
        StarY[i] = Random(182) + 18;
        StarMove[i] = Random(MaxSpeed) + MinSpeed;
    }
    for (i = 1; i <= MaxMeteoritStar; i++) {
        OldMStarX[i] = 2000;
        OldMStarY[i] = 1000;
    }
    x = MinX; y = 140;
    ShipMoveY = 0; ShipMoveX = 0;
    Taste = 0;
    StarMaxSpeed = MaxSpeed;
    Shoot = 0; ShootCount = 0;
    Meteor = 0;
    for (i = 1; i <= MaxUfoAnz; i++) UfoOnScreen[i] = 0;
    BarrelOnScreen = 0;
    EnemyShootOnScreen = 0;
    WuerfelOnScreen = 0;
    BrockenOnScreen = 0;
    Score = 0;
    Fuel = 312;
    MeteorDestroyed = 0;
    FuelCount = MaxFuelCount;
    UfoRandom = UfoRandomAnf;
    WuerfelRandom = WuerfelRandomAnf;
    BrockenRandom = BrockenRandomAnf;
    EnemyRandom = EnemyRandomAnf;
    EnemyShootRandom = EnemyShootRandomAnf;
    MaxBrockenAnz = MaxBrockenAnzAnf;
    EnemyOnScreen = 0;
}

/* ---------------------------------------------------------------------- */
/* Meteoritensplitter                                                     */
/* ---------------------------------------------------------------------- */
static void MoveMeteoritStars(void) {
    int i;
    for (i = 1; i <= MaxMeteoritStar; i++) {
        if (MStarX[i] != 0) {
            PutPixel(OldMStarX[i] >> 2, OldMStarY[i] >> 2, 0);
            if (GetPixel(MStarX[i] >> 2, MStarY[i] >> 2) == 0 && MStarY[i] > 68) {
                PutPixel(MStarX[i] >> 2, MStarY[i] >> 2, 3);
                OldMStarX[i] = MStarX[i];
                OldMStarY[i] = MStarY[i];
            } else OldMStarX[i] = 1280;
            MStarX[i] += MStarMoveX[i];
            MStarY[i] += MStarMoveY[i];
            if (MStarX[i] < 4 || MStarX[i] > 1280 || MStarY[i] < 68 || MStarY[i] > 800) {
                PutPixel(OldMStarX[i] >> 2, OldMStarY[i] >> 2, 0);
                MStarX[i] = 0;
            }
        }
    }
    MeteorDestroyedCount = Pred(MeteorDestroyedCount);
    if (MeteorDestroyedCount == 0) {
        MeteorDestroyed = 0;
        for (i = 1; i <= MaxMeteoritStar; i++)
            PutPixel(OldMStarX[i] >> 2, OldMStarY[i] >> 2, 0);
    }
}

static void MeteoritDestroyed(void) {
    int i;
    if (SoundOn) Sound(100);
    for (i = 1; i <= MaxMeteoritStar; i++) {
        MStarX[i] = (mx + 12) << 2;
        MStarY[i] = (my + 12) << 2;
        MStarMoveX[i] = Random(33) - 16;
        MStarMoveY[i] = Random(33) - 16;
    }
    if (SoundOn) Sound(200);
    MeteorDestroyed = 1;
    MeteorDestroyedCount = 100;
    mx = 0;
    if (SoundOn) Sound(100);
    MoveMeteoritStars();
    NoSound();
}

/* ---------------------------------------------------------------------- */
/* Schuss-Kollisionen                                                     */
/* ---------------------------------------------------------------------- */
/* Im Pixelmodus zusaetzlich pruefen, ob der Schuss (Punkt px,py) wirklich
   ein nicht-schwarzes Pixel des Objekt-Sprites trifft. */
static int shot_pix(const unsigned char *img, int ox, int oy, int px, int py) {
    return !PixelCollision || gfx_sprite_pixel(img, px - ox, py - oy);
}

static void ShootAtMeteor(void) {
    int ShootFlag = 0;
    if (ShootY[j] > my && ShootY[j] < my + 24) {
        for (p = ShootX[j]; p <= ShootX[j] + ShootAdd; p++)
            if (p < mx + 24 && p > mx && shot_pix(Meteorit, mx, my, p, ShootY[j])) {
                ShootFlag = 1;
                p = ShootX[j] + ShootAdd;
                SetColor(0);
                Line(ShootX[j], ShootY[j], ShootX[j] + ShootAdd, ShootY[j]);
                ShootX[j] = 319;
            }
    }
    if (ShootFlag) {
        Meteor = 0;
        PutImage(mx, my, MeteoritLoesch);
        Score += (300 - (mx - x)) / 2;
        PrintScore();
        MeteorDestroyed = 1;
        MeteoritDestroyed();
    }
}

static void ShootAtBarrel(void) {
    int ShootFlag = 0;
    if (ShootY[j] > by && ShootY[j] < by + 24) {
        for (p = ShootX[j]; p <= ShootX[j] + ShootAdd; p++)
            if (p < bx + 13 && p > bx && shot_pix(Barrel, bx, by, p, ShootY[j])) {
                ShootFlag = 1;
                p = ShootX[j] + ShootAdd;
                SetColor(0);
                Line(ShootX[j], ShootY[j], ShootX[j] + ShootAdd, ShootY[j]);
                ShootX[j] = 319;
            }
    }
    if (ShootFlag) {
        if (SoundOn) Sound(300);
        BarrelOnScreen = 0;
        Bar(bx, by, bx + 24, by + 13);
        Score += (long)(300 - (bx - x)) * 2;
        PrintScore();
        Fuel = 312;
        draw_fuel_bar_full();
        NoSound();
    }
}

static void ShootAtWuerfel(void) {
    int ShootFlag = 0;
    if (ShootY[j] > wy && ShootY[j] < wy + 22) {
        for (p = ShootX[j]; p <= ShootX[j] + ShootAdd; p++)
            if (p < wx + 28 && p > wx && shot_pix(Wuerfel[WuerfelCount >> 3], wx, wy, p, ShootY[j])) {
                ShootFlag = 1;
                p = ShootX[j] + ShootAdd;
                SetColor(0);
                Line(ShootX[j], ShootY[j], ShootX[j] + ShootAdd, ShootY[j]);
                ShootX[j] = 319;
            }
    }
    if (ShootFlag) {
        if (Lives < 6) {
            if (SoundOn) Sound(900);
            WuerfelOnScreen = 0;
            Bar(wx, wy, wx + 28, wy + 22);
            Score += (long)(300 - (wx - x)) * 2;
            PrintScore();
            if (SoundOn) Sound(450);
            Lives = Succ(Lives);
            if (SoundOn) Sound(225);
            PrintLives();
            NoSound();
        }
    }
}

static void ShootAtUfo(void) {
    for (i = 1; i <= MaxUfoAnz; i++)
        if (UfoOnScreen[i]) {
            int ShootFlag = 0;
            if (ShootY[j] > uy[i] - 3 && ShootY[j] < uy[i] + 11) {
                for (p = ShootX[j]; p <= ShootX[j] + ShootAdd; p++)
                    if (p < ux[i] + 31 && p > ux[i] && shot_pix(Ufo[Ufonr], ux[i], uy[i], p, ShootY[j])) {
                        ShootFlag = 1;
                        p = ShootX[j] + ShootAdd;
                        SetColor(0);
                        Line(ShootX[j], ShootY[j], ShootX[j] + ShootAdd, ShootY[j]);
                        ShootX[j] = 319;
                    }
            }
            if (ShootFlag) {
                if (SoundOn) Sound(35);
                UfoOnScreen[i] = 0;
                PutImage(ux[i], uy[i], UfoLoesch);
                Score += (300 - (ux[i] - x));
                PrintScore();
                NoSound();
            }
        }
}

static void ShootAtEnemy(void) {
    int ShootFlag = 0;
    if (ShootY[j] > ey && ShootY[j] < ey + 23) {
        for (p = ShootX[j]; p <= ShootX[j] + ShootAdd; p++)
            if (p < ex + 39 && p > ex && shot_pix(Ship1, ex, ey, p, ShootY[j])) {
                ShootFlag = 1;
                p = ShootX[j] + ShootAdd;
                SetColor(0);
                Line(ShootX[j], ShootY[j], ShootX[j] + ShootAdd, ShootY[j]);
                ShootX[j] = 319;
            }
    }
    if (ShootFlag) {
        if (SoundOn) Sound(100);
        EnemyOnScreen = 0;
        Bar(ex, ey, ex + 39, ey + 23);
        Score += (long)(300 - (mx - x)) * 2;
        PrintScore();
        NoSound();
    }
}

static void MoveShoot(void) {
    int k, ShootGone = 0;
    NoSound();
    for (j = 1; j <= ShootCount; j++) {
        if (Meteor) ShootAtMeteor();
        if (BarrelOnScreen) ShootAtBarrel();
        if (WuerfelOnScreen) ShootAtWuerfel();
        ShootAtUfo();
        if (EnemyOnScreen) ShootAtEnemy();
        ShootX[j] += ShootAdd;
        SetColor(0);
        Line(ShootX[j] - ShootAdd, ShootY[j], ShootX[j], ShootY[j]);
        if (ShootX[j] < 319) {
            SetColor(2);
            Line(ShootX[j], ShootY[j], ShootX[j] + ShootAdd, ShootY[j]);
        } else ShootGone = 1;
    }
    if (ShootGone) {
        k = ShootCount;
        for (j = 1; j <= k; j++) {
            if (ShootX[j] > 319) {
                ShootX[j] = ShootX[ShootCount];
                ShootY[j] = ShootY[ShootCount];
                ShootCount = Pred(ShootCount);
            }
        }
    }
    if (ShootCount == 0) Shoot = 0;
}

/* ---------------------------------------------------------------------- */
/* Steuerung                                                              */
/* ---------------------------------------------------------------------- */
static void ControlKeys(void) {
    Taste = Key();
    switch (Taste) {
        case 72: ShipMoveY = -3; ShipMoveX = 0; break;
        case 80: ShipMoveY =  3; ShipMoveX = 0; break;
        case 75: ShipMoveX = -3; ShipMoveY = 0; break;
        case 77: ShipMoveX =  3; ShipMoveY = 0; break;
        case 71: ShipMoveX = -3; ShipMoveY = -3; break;
        case 73: ShipMoveX =  3; ShipMoveY = -3; break;
        case 79: ShipMoveX = -3; ShipMoveY =  3; break;
        case 81: ShipMoveX =  3; ShipMoveY =  3; break;
        case 57:
            if (ShootCount < MaxShootAnz) {
                if (SoundOn) Sound(60);
                Shoot = 1;
                Fuel = Pred(Fuel);
                PrintFuel();
                ShootCount = Succ(ShootCount);
                ShootX[ShootCount] = x + 58;
                ShootY[ShootCount] = y + 15;
            }
            break;
        case 1:
            GetImage(0, 0, 319, 199, Puffer);
            SetUserCharSize(10, 16, 12, 16);
            SetTextJustify(GFX_LEFT, GFX_LEFT);
            OutTextXY3D(60, 70, "pausing . . .", 1, 3);
            OutTextXY3D(10, 120, "press ESC to finish game or", 2, 3);
            OutTextXY3D(10, 150, "any other key to continue !", 3, 3);
            ClearInput();
            a = (char)ReadKey();
            if (a != 27) Taste = 0;
            SetScreenMode();
            PutImage(0, 0, Puffer);
            break;
        default:
            ShipMoveX = 0;
            ShipMoveY = 0;
            break;
    }
}

static void MoveStars(void) {
    int i;
    for (i = 1; i <= MaxStar; i++) {
        PutPixel(OldStarX[i] >> 3, StarY[i], 0);
        if (GetPixel(StarX[i] >> 3, StarY[i]) == 0) {
            PutPixel(StarX[i] >> 3, StarY[i], 3);
            OldStarX[i] = StarX[i];
        } else OldStarX[i] = 2560;
        StarX[i] -= StarMove[i];
        if (StarX[i] < 0) {
            PutPixel((StarX[i] + StarMove[i]) >> 3, StarY[i], 0);
            StarX[i] = Random(120) + 2440;
            StarY[i] = Random(182) + 18;
            StarMove[i] = Random(StarMaxSpeed) + MinSpeed;
        }
    }
}

static void MoveShip(void) {
    if (ShipMoveY != 0) {
        if (y < MinY && ShipMoveY < 0) ShipMoveY = 0;
        if (y > MaxY && ShipMoveY > 0) ShipMoveY = 0;
        y += ShipMoveY;
    }
    if (ShipMoveX != 0) {
        if (x < MinX && ShipMoveX < 0) ShipMoveX = 0;
        if (x > MaxX && ShipMoveX > 0) ShipMoveX = 0;
        x += ShipMoveX;
    }
    PutImageT(x, y, Jet);
}

static void MakeMeteor(void) {
    if (Random(MeteorRandom) == 0) {
        Meteor = 1;
        mx = 296;
        my = Random(162) + 18;
        mxadd = Succ(Random(3));
        PutImageT(mx, my, Meteorit);
    }
}

static void InitJetStars(void) {
    int i;
    for (i = 1; i <= MaxJetStar; i++) {
        JStarX[i] = (sx + Random(58) - 25) << 2;
        JStarY[i] = (sy + Random(30) - 15) << 2;
        JStarMoveX[i] = Random(45) - 22;
        JStarMoveY[i] = Random(45) - 22;
        OldJStarX[i] = 1290;
    }
}

static void MoveShipDestroyedStars(void) {
    int i;
    for (i = 1; i <= MaxJetStar; i++) {
        PutPixel(x + Random(58), y + Random(30), 0);
        PutPixel(OldJStarX[i] >> 2, OldJStarY[i] >> 2, 0);
        if (GetPixel(JStarX[i] >> 2, JStarY[i] >> 2) == 0) {
            PutPixel(JStarX[i] >> 2, JStarY[i] >> 2, Succ(Random(3)));
            OldJStarX[i] = JStarX[i];
            OldJStarY[i] = JStarY[i];
        } else OldJStarX[i] = 1290;
        JStarX[i] += JStarMoveX[i];
        JStarY[i] += JStarMoveY[i];
        if (JStarX[i] < 4 || JStarX[i] > 1280 || JStarY[i] < 76 || JStarY[i] > 800) {
            PutPixel(OldJStarX[i] >> 2, OldJStarY[i] >> 2, 0);
            JStarX[i] = sx << 2;
            JStarY[i] = sy << 2;
            JStarMoveX[i] = (Random(45) - 22) * 2;
            JStarMoveY[i] = (Random(45) - 22) * 2;
            if (JStarMoveX[i] == 0) JStarMoveX[i] = 9;
        }
    }
}

static void GameEnd(void);   /* fwd */

static void ShipLost(void) {
    int i;
    sx = x + 25;
    sy = y + 15;
    InitJetStars();
    for (i = 1; i <= 25; i++) {
        if (SoundOn) Sound(50 + Random(50) - i * 2);
        PutPixel(x + Random(58), y + Random(30), 0);
        MoveShipDestroyedStars();
        MoveStars();
        PutPixel(x + Random(58), y + Random(30), 0);
        if (SoundOn) Sound(50 + Random(50) - i * 2);
        frame();                    /* Animation sichtbar machen */
    }
    for (i = 1; i <= MaxJetStar; i++)
        PutPixel(OldJStarX[i] >> 2, OldJStarY[i] >> 2, 0);
    NoSound();
    Lives = Pred(Lives);
    DelLive();
    if (Lives == 0) GameEnd();
}

/* true, wenn das Schiff das Objekt trifft: im Pixelmodus zusaetzlich zum
   (guenstigen) Rechteck-Vortest die pixelgenaue Sprite-Ueberlappung. */
static int ship_hits(const unsigned char *obj, int ox, int oy) {
    if (!PixelCollision) return 1;
    return gfx_sprites_collide(Jet, x, y, obj, ox, oy);
}

static void ControlCrash(void) {
    int i;
    if (Meteor) {
        if (mx + 16 > x && mx - 54 < x)
            if (my + 19 > y && my - 24 < y)
                if (ship_hits(Meteorit, mx, my)) {
                    ShipLost();
                    PutImage(mx, my, MeteoritLoesch);
                    MeteoritDestroyed();
                    PutImageT(x, y, Jet);
                }
    }
    for (i = 1; i <= MaxUfoAnz; i++) {
        if (UfoOnScreen[i]) {
            if (ux[i] + 22 > x && ux[i] - 51 < x)
                if (uy[i] + 10 > y && uy[i] - 25 < y)
                    if (ship_hits(Ufo[Ufonr], ux[i], uy[i])) {
                        ShipLost();
                        PutImage(ux[i], uy[i], UfoLoesch);
                        UfoOnScreen[i] = 0;
                        PutImageT(x, y, Jet);
                    }
        }
    }
    if (BrockenOnScreen) {
        for (i = 1; i <= BrockenAnz; i++) {
            if (BrockenX[i] + 18 > x && BrockenX[i] - 51 < x)
                if (BrockenY[i] + 10 > y && BrockenY[i] - 25 < y)
                    if (ship_hits(Brocken[BrockenNr[i]], BrockenX[i], BrockenY[i])) {
                        ShipLost();
                        Bar(BrockenX[i], BrockenY[i], BrockenX[i] + 29, BrockenY[i] + 13);
                        BrockenX[i] = -4;
                        BrockenY[i] = 400;
                        PutImageT(x, y, Jet);
                    }
        }
    }
}

static void MoveMeteor(void) {
    mx -= mxadd;
    if (mx > 0) PutImageT(mx, my, Meteorit);
    else {
        Meteor = 0;
        mx += mxadd;
        PutImage(mx, my, MeteoritLoesch);
    }
}

static void MakeUfo(void) {
    for (i = 1; i <= MaxUfoAnz; i++) {
        if (UfoOnScreen[i] == 0)
            if (Random(UfoRandom) == 0) {
                UfoOnScreen[i] = 1;
                ux[i] = 300;
                uy[i] = Random(162) + 18;
                uxadd[i] = -Succ(Random(5));
                uyadd[i] = Random(3);
                UfoCount = 0;
                Ufonr = 0;
                PutImageT(ux[i], uy[i], Ufo[Ufonr]);
            }
    }
}

static void MoveUfo(void) {
    int i;
    UfoCount = Succ(UfoCount);
    if (UfoCount == 10) UfoCount = 0;
    Ufonr = UfoCount >> 1;
    for (i = 1; i <= MaxUfoAnz; i++) {
        if (UfoOnScreen[i]) {
            ux[i] += uxadd[i];
            if (ux[i] > 0) PutImageT(ux[i], uy[i], Ufo[Ufonr]);
            else {
                UfoOnScreen[i] = 0;
                ux[i] -= uxadd[i];
                PutImage(ux[i], uy[i], UfoLoesch);
            }
        }
        if (y > uy[i]) uy[i] += uyadd[i];
        else if (y < uy[i]) uy[i] -= uyadd[i];
    }
}

static void MakeBrocken(void) {
    BrockenOnScreen = 1;
    BrockenAnz = Succ(Random(MaxBrockenAnz));
    for (i = 1; i <= BrockenAnz; i++) {
        BrockenX[i] = 300 + Random(60);
        BrockenY[i] = Random(162) + 18;
        BrockenmoveX[i] = -Succ(Random(4));
        BrockenNr[i] = Random(4);
        Brockenweg[i] = 0;
        PutImageT(BrockenX[i], BrockenY[i], Brocken[BrockenNr[i]]);
    }
    BrockenCount = BrockenAnz;
}

static void MoveBrocken(void) {
    int i;
    for (i = 1; i <= BrockenAnz; i++) {
        BrockenX[i] += BrockenmoveX[i];
        if (BrockenX[i] > 0) PutImageT(BrockenX[i], BrockenY[i], Brocken[BrockenNr[i]]);
        else {
            if (Brockenweg[i] == 0) {
                Bar(BrockenX[i], BrockenY[i], BrockenX[i] + 20, BrockenY[i] + 13);
                BrockenCount = Pred(BrockenCount);
                if (BrockenCount == 0) BrockenOnScreen = 0;
                Brockenweg[i] = 1;
            }
        }
    }
}

/* ---------------------------------------------------------------------- */
/* Gegner-Schuss                                                          */
/* ---------------------------------------------------------------------- */
static void ShootAtSpaceShip(void) {
    int ShootFlag = 0;
    if (esy2 > y && esy1 < y + 29) {
        for (p = esx; p <= esx + exadd; p++)
            if (p < x + 39 && p > x &&
                (!PixelCollision ||
                 gfx_sprite_pixel(Jet, p - x, esy1 - y) ||
                 gfx_sprite_pixel(Jet, p - x, esy2 - y))) {
                ShootFlag = 1;
                p = esx + exadd;
                SetColor(0);
                Line(esx + exadd, esy1, esx, esy1);
                Line(esx + exadd, esy2, esx, esy2);
                esx = -2;
            }
    }
    if (ShootFlag) {
        if (SoundOn) Sound(70);
        ShipLost();
        NoSound();
    }
}

static void EnemyShoots(void) {
    esy1 = ey + 5;
    esy2 = ey + 17;
    exadd = EnemyShootAdd + Random(3);
    esx = ex;
    SetColor(2);
    Line(esx + exadd, esy1, esx, esy1);
    Line(esx + exadd, esy2, esx, esy2);
    EnemyShootOnScreen = 1;
}

static void MoveEnemyShoot(void) {
    SetColor(0);
    Line(esx + exadd, esy1, esx, esy1);
    Line(esx + exadd, esy2, esx, esy2);
    esx -= exadd;
    if (esx > 0) {
        SetColor(Succ(Random(3)));
        Line(esx + exadd, esy1, esx, esy1);
        Line(esx + exadd, esy2, esx, esy2);
        ShootAtSpaceShip();
    } else EnemyShootOnScreen = 0;
}

static void MakeEnemy(void) {
    EnemyOnScreen = 1;
    ex = 290;
    ey = Random(162) + 18;
    exmove = -Succ(Random(5));
    eymove = Random(3);
    PutImageT(ex, ey, Ship1);
}

static void MoveEnemy(void) {
    if (ex > x) ex += exmove; else ex -= 1;
    if (ex > 0) PutImageT(ex, ey, Ship1);
    else {
        EnemyOnScreen = 0;
        ex -= exmove;
        Bar(ex, ey, ex + 39, ey + 23);
    }
    if (y > ey) ey += eymove;
    else if (y < ey) ey -= eymove;
    if (EnemyShootOnScreen == 0)
        if (Random(EnemyShootRandom) == 0) EnemyShoots();
}

/* ---------------------------------------------------------------------- */
/* Fass / Wuerfel / Sprit                                                 */
/* ---------------------------------------------------------------------- */
static void InitBarrel(void) {
    bx = 310;
    by = Random(150) + 26;
    bmove = Succ(Random(5));
    PutImageT(bx, by, Barrel);
    BarrelOnScreen = 1;
}

static void MoveBarrel(void) {
    bx -= bmove;
    if (bx < 0) {
        BarrelOnScreen = 0;
        Bar(bx, by, bx + 24, by + 13);
    } else PutImageT(bx, by, Barrel);
}

static void MakeWuerfel(void) {
    wx = 300;
    wy = Random(150) + 26;
    wxmove = Succ(Random(5));
    WuerfelCount = Random(32);
    PutImageT(wx, wy, Wuerfel[WuerfelCount >> 3]);
    WuerfelOnScreen = 1;
}

static void MoveWuerfel(void) {
    wx -= wxmove;
    WuerfelCount = Succ(WuerfelCount);
    if (WuerfelCount == MaxWuerfelCount) WuerfelCount = 0;
    if (wx < 0) {
        WuerfelOnScreen = 0;
        Bar(wx, wy, wx + 28, wy + 22);
    } else PutImageT(wx, wy, Wuerfel[WuerfelCount >> 3]);
}

static void ConsumeFuel(void) {
    FuelCount = Pred(FuelCount);
    if (FuelCount == 0) {
        Fuel = Pred(Fuel);
        PrintFuel();
        if (Fuel <= 0) {
            ShipLost();
            Fuel = 312;
            draw_fuel_bar_full();
        }
        FuelCount = MaxFuelCount;
    }
}

/* ---------------------------------------------------------------------- */
/* GameEnd + GotHiScore                                                   */
/* ---------------------------------------------------------------------- */
static void GotHiScore(void) {
    long r;
    char hstr[32];
    SetScreenMode();
    SetTextJustify(GFX_LEFT, GFX_LEFT);
    SetColor(3);
    ClearInput();
    Box(0, 0, 319, 199);
    SetColor(2);
    Box(1, 1, 318, 198);
    r = HiVal[8];
    sprintf(hstr, "%ld", Score);
    SetUserCharSize(11, 16, 10, 16);
    SetColor(1);
    OutTextXY(9, 19, "All your ships are destroyed.");
    SetUserCharSize(8, 16, 10, 16);
    SetColor(2);
    OutTextXY(9, 39, "You have achieved");
    {
        char pts[48];
        sprintf(pts, "%ld points.", Score);
        OutTextXY3D(157, 38, pts, 1, 3);
    }
    if (Score >= r) {
        int ins, k;
        SetColor(1);
        SetUserCharSize(9, 16, 10, 16);
        OutTextXY(19, 70, "Therefore you get a place in the");
        OutTextXY(19, 88, "Hiscore table. Please enter your");
        OutTextXY(19, 106, "name (up to 12 characters) .");
        SetColor(2);
        OutTextXY(9, 160, "Your name : ");
        SetColor(3);
        GraphRead(125, 160, Nameein);
        ins = 9;
        for (k = 1; k <= 8; k++) if (Score >= HiVal[k]) { ins = k; break; }
        if (ins <= 8) {
            for (k = 8; k > ins; k--) {
                HiVal[k] = HiVal[k-1];
                strcpy(HiName[k], HiName[k-1]);
            }
            HiVal[ins] = Score;
            snprintf(HiName[ins], sizeof HiName[ins], "%s", Nameein);
        }
    } else {
        SetUserCharSize(10, 16, 12, 16);
        OutTextXY(14, 183, "press any key to continue...");
        a = (char)ReadKey();
    }
}

static void GameEnd(void) {
    int i;
    SetColor(1);
    for (i = 169; i >= 0; i--) {
        MoveTo(i, (int)(i / 1.6));
        LineTo(i, 199 - (int)(i / 1.6));
        LineTo(319 - i, 199 - (int)(i / 1.6));
        LineTo(319 - i, (int)(i / 1.6));
        LineTo(i, (int)(i / 1.6));
    }
    GotHiScore();
    while (Keypressed()) a = (char)ReadKey();
}

/* ---------------------------------------------------------------------- */
/* Hauptspiel (StartGame)                                                 */
/* ---------------------------------------------------------------------- */
static void StartGame(void) {
    g_scene = 3;
    SetScreenMode();
    InitGame();
    InitScreen();
    PutImageT(x, y, Jet);
    g_last_frame = plat_time();
    do {
        Bar(0, 18, 319, 199);          /* Spielfeld leeren (HUD y<18 bleibt) */
        MoveStars();
        if (Keypressed()) ControlKeys();
        if (ShipMoveY != 0 || ShipMoveX != 0) MoveShip();
        else PutImageT(x, y, Jet);
        if (Shoot) MoveShoot();
        if (Meteor == 0 && MeteorDestroyed == 0) MakeMeteor();
        if (Meteor) MoveMeteor();
        else if (MeteorDestroyed) MoveMeteoritStars();
        if (BarrelOnScreen) MoveBarrel();
        else if (Random(BarrelRandom) == 0) InitBarrel();
        if (EnemyOnScreen) MoveEnemy();
        else if (Random(EnemyRandom) == 0) MakeEnemy();
        if (WuerfelOnScreen) MoveWuerfel();
        else if (Random(WuerfelRandom) == 0) MakeWuerfel();
        if (BrockenOnScreen) MoveBrocken();
        else if (Random(BrockenRandom) == 0) MakeBrocken();
        if (EnemyShootOnScreen) MoveEnemyShoot();
        MoveUfo();
        MakeUfo();
        ConsumeFuel();
        ControlCrash();
        frame();                       /* ersetzt Delay(SpeedDelay) */
    } while (g_running && Taste != 1 && Lives != 0);
    ClearInput();
}

/* ---------------------------------------------------------------------- */
/* HiScore-Anzeige, Titel, Menue, Setup, Erklaerung, Ende                 */
/* ---------------------------------------------------------------------- */
static void HiScoreScreen(void) {
    char hstr[8];
    Bar(0, 0, 319, 199);
    SetColor(3); Box(0, 0, 319, 199);
    SetColor(2); Box(1, 1, 318, 198);
    SetTextJustify(GFX_LEFT, GFX_LEFT);
    SetUserCharSize(12, 16, 9, 16);
    SetColor(1); OutTextXY(11, 16, "Spaceflight");
    SetColor(2); OutTextXY(12, 17, "Spaceflight  Hiscores :");
    SetColor(3); OutTextXY(13, 18, "Spaceflight");
    SetUserCharSize(9, 16, 8, 16);
    for (i = 1; i <= 8; i++) {
        char sc[16];
        SetColor(Succ(i % 3));
        sprintf(hstr, "%d.", i);
        OutTextXY(15, 25 + i * 18, hstr);
        OutTextXY(40, 25 + i * 18, HiName[i]);
        sprintf(sc, "%8ld", HiVal[i]);
        OutTextXY(210, 25 + i * 18, sc);
    }
    SetColor(2);
    SetUserCharSize(10, 16, 8, 16);
    OutTextXY(10, 186, "press any key to continue...");
    sx = 160; sy = 100;
    InitPageStars();
    wait_stars();
}

static void TitleScreen(void) {
    /* Originalgetreu (SPACDATA.PAS): 16 Groessen des "ABCS"-Logos werden
       vorgerendert; danach huepft das pulsierende Logo per PutImage ueber
       den Bildschirm (der opake 81x39-Block loescht dabei seine Spur),
       waehrend die Seiten-Sterne animieren. */
    static unsigned char BildPuffer[17][900];
    const int MaxSchriftGroesse = 16;
    const int oben = 1, links = 1, unten = 131, rechts = 238;
    int SizeAdd, BildSize, xpos, ypos, xadd, yadd, zxpos, zypos, k;

    g_scene = 1;
    SetScreenMode();
    for (k = 1; k <= MaxSchriftGroesse; k++) {
        Bar(120, 52, 200, 90);
        SetUserCharSize(k, 16, k, 16);
        SetTextJustify(GFX_CENTER, GFX_CENTER);
        SetColor(Succ(k % 3));
        OutTextXY3D(159, 67, "ABCS", Succ(k % 3), 3);
        GetImage(120, 52, 200, 90, BildPuffer[k]);
    }
    Bar(120, 52, 200, 90);
    SetColor(3); Box(0, 0, 319, 199);
    SetColor(2); Box(1, 1, 318, 198);
    SetTextJustify(GFX_LEFT, GFX_LEFT);
    SetUserCharSize(9, 16, 9, 16);
    SetColor(2); OutTextXY(10, 186, "ABCS presents :");
    SetUserCharSize(14, 16, 12, 16);
    /* y = Oberkante (Top-Left); tiefer positioniert wuerde der 3D-Schatten
       unten aus dem 200px-Bild laufen -> hoeher setzen, damit alles passt. */
    OutTextXY3D(150, 176, "Spaceflight", 1, 6);

    BildSize = 1; SizeAdd = 1;
    xpos = 4 + Random(100); ypos = 4 + Random(100);
    xadd = 1; yadd = 1;
    sx = 100; sy = 70;
    InitPageStars();
    while (g_running && !Keypressed()) {
        PutImage(xpos, ypos, BildPuffer[BildSize]);
        BildSize += SizeAdd;
        if (BildSize <= 1) SizeAdd = 1;
        if (BildSize >= MaxSchriftGroesse) SizeAdd = -1;
        zxpos = xpos; xpos += xadd;
        if (xpos <= links || xpos >= rechts) { xadd = -xadd; xpos = zxpos; }
        zypos = ypos; ypos += yadd;
        if (ypos <= oben || ypos >= unten) { yadd = -yadd; ypos = zypos; }
        MovePageStars();
        frame();
    }
    a = (char)ReadKey();
}

static int MainMenu(void) {
    int wahl = 0;
    g_scene = 2;
    SetScreenMode();
    SetTextJustify(GFX_LEFT, GFX_LEFT);
    SetColor(3); Box(0, 0, 319, 199);
    SetColor(2); Box(1, 1, 318, 198);
    SetUserCharSize(14, 16, 12, 16);
    OutTextXY3D(83, 25, "Spaceflight", 1, 6);
    SetUserCharSize(8, 16, 8, 16);
    OutTextXY3D(10, 59, "written 1989 by Andreas Bauer (ABCS)", 1, 3);
    SetUserCharSize(11, 16, 11, 16);
    SetColor(3); OutTextXY(8, 85, "Main Menu :");
    SetColor(2);
    SetUserCharSize(13, 16, 9, 16);
    OutTextXY(8, 105, "-1- Show hiscores");
    OutTextXY(8, 120, "-2- Start game");
    OutTextXY(8, 135, "-3- Explain game");
    OutTextXY(8, 150, "-4- Setup");
    OutTextXY(8, 165, "-5- Quit game");
    SetUserCharSize(11, 16, 11, 16);
    OutTextXY3D(15, 185, "please make your choice...", 1, 3);
    sx = 190; sy = 70;
    InitPageStars();
    while (g_running) {
        MovePageStars();
        if (Keypressed()) {
            a = (char)ReadKey();
            if (ValChar(a, &wahl) != 0) wahl = 0;
        }
        if (wahl > 0 && wahl < 6) break;
        frame();
    }
    if (!g_running) return 5;
    return wahl;
}

static void setup_val(int y, const char *s, int col) {
    SetUserCharSize(11, 16, 11, 16);
    Bar(165, y - 3, 305, y + 16);
    OutTextXY3D(170, y, s, col, 3);
}
static void setup_draw_mode(void) {
    setup_val(114, g_cfg.fullscreen ? "FULLSCREEN" : "WINDOW", 1);
}
static void setup_draw_size(void) {
    char buf[24];
    sprintf(buf, "%dx%d", g_res_presets[g_res_index][0], g_res_presets[g_res_index][1]);
    setup_val(136, buf, 2);
}

static void Setup(void) {
    int wahl = 0, Check;
    char hstr[16];
    g_scene = 4;
    Bar(2, 2, 317, 197);
    SetTextJustify(GFX_CENTER, GFX_CENTER);
    SetUserCharSize(15, 16, 15, 16);
    OutTextXY3D(158, 16, "Setup", 1, 3);
    SetTextJustify(GFX_LEFT, GFX_LEFT);
    SetUserCharSize(11, 16, 11, 16);
    SetColor(2); OutTextXY(10, 48, "-1- Sound :");
    if (SoundOn) OutTextXY3D(170, 48, "ON", 1, 3);
    else OutTextXY3D(170, 48, "OFF", 1, 3);
    sprintf(hstr, "%d", SpeedDelay);
    SetColor(2); OutTextXY(10, 70, "-2- Speed :");
    OutTextXY3D(170, 70, hstr, 2, 3);
    SetColor(2); OutTextXY(10, 92, "-3- Collision :");
    if (PixelCollision) OutTextXY3D(170, 92, "PIXEL", 1, 3);
    else OutTextXY3D(170, 92, "BOX", 1, 3);
    SetColor(2); OutTextXY(10, 114, "-4- Mode :");
    setup_draw_mode();
    SetColor(2); OutTextXY(10, 136, "-5- Win-Size :");
    setup_draw_size();
    SetColor(2); OutTextXY(10, 158, "-6- Main Menu");
    SetUserCharSize(10, 16, 10, 16);
    OutTextXY3D(15, 184, "please make your choice...", 1, 3);
    sx = 159; sy = 100;
    InitPageStars();
    do {
        wahl = 0;
        while (g_running) {
            MovePageStars();
            if (Keypressed()) {
                a = (char)ReadKey();
                if (ValChar(a, &wahl) != 0) wahl = 0;
            }
            if (wahl > 0 && wahl < 7) break;
            frame();
        }
        if (!g_running) return;
        if (wahl == 1) {
            SoundOn = !SoundOn;
            setup_val(48, SoundOn ? "ON" : "OFF", 1);
        } else if (wahl == 2) {
            do {
                Bar(165, 68, 305, 90);
                GraphRead(170, 70, hstr);
                SpeedDelay = atoi(hstr);
                Check = (hstr[0] == 0) ? 1 : 0;
            } while (Check != 0 && g_running);
        } else if (wahl == 3) {
            PixelCollision = !PixelCollision;
            setup_val(92, PixelCollision ? "PIXEL" : "BOX", 1);
        } else if (wahl == 4) {
            g_cfg.fullscreen = !g_cfg.fullscreen;
            if (g_cfg.fullscreen) { g_cfg.width = 0; g_cfg.height = 0; }
            else { g_cfg.width = g_res_presets[g_res_index][0];
                   g_cfg.height = g_res_presets[g_res_index][1]; }
            apply_display();
            setup_draw_mode();
            setup_draw_size();
        } else if (wahl == 5) {
            g_res_index = (g_res_index + 1) % N_RES;
            if (!g_cfg.fullscreen) {
                g_cfg.width = g_res_presets[g_res_index][0];
                g_cfg.height = g_res_presets[g_res_index][1];
                apply_display();
            }
            setup_draw_size();
        }
    } while (wahl != 6 && g_running);
    SaveHighScores();          /* Anzeige-/Spieloptionen sofort sichern */
}

static void info_page_wait(void) { sx = 160; sy = 100; InitPageStars(); wait_stars(); }

/* Vollstaendiger Anleitungstext, 7 Seiten wie im Original (SPACDATA.PAS).
   Schriftgroesse so gewaehlt (8/16), dass jede Zeile wie im Original passt. */
static void explain_page_begin(void) {
    Bar(2, 2, 317, 197);
    SetUserCharSize(8, 16, 8, 16);
}

static void ExplainGame(void) {
    g_scene = 5;
    /* Seite 1 */
    explain_page_begin();
    SetTextJustify(GFX_CENTER, GFX_CENTER);
    OutTextXY3D(158, 15, "How to play SPACEFLIGHT", 1, 3);
    OutTextXY3D(159, 50, "You have to move the spaceliner AEROFOX", 1, 2);
    OutTextXY3D(159, 65, "through the endless width of the space", 1, 2);
    OutTextXY3D(159, 80, "to bring important medicaments to the", 1, 2);
    OutTextXY3D(159, 95, "planet ENOLA. But there are lots of", 1, 2);
    OutTextXY3D(159, 110, "meteorits flying around which destroy", 1, 2);
    OutTextXY3D(159, 125, "your ship if you get too close.", 1, 2);
    OutTextXY3D(159, 140, "Another danger to your spaceship are", 1, 2);
    OutTextXY3D(159, 155, "the pirates who attack every trader,and", 1, 2);
    OutTextXY3D(159, 170, "UFOs which approach foreign spaceships", 1, 2);
    OutTextXY3D(159, 185, "in order to exstinguish them.", 1, 2);
    info_page_wait();

    /* Seite 2 */
    explain_page_begin();
    SetTextJustify(GFX_CENTER, GFX_CENTER);
    OutTextXY3D(158, 15, "Keys you need for ship control", 1, 3);
    OutTextXY3D(159, 50, "You can use all cursorkeys including", 1, 2);
    OutTextXY3D(159, 65, "Home,PgUp,End and PgDn to move the", 1, 2);
    OutTextXY3D(159, 80, "spaceship. The space-key is used", 1, 2);
    OutTextXY3D(159, 95, "to fire a shoot.", 1, 2);
    OutTextXY3D(159, 110, "After pressing ESC the game will", 1, 2);
    OutTextXY3D(159, 125, "be paused and you can continue", 1, 2);
    OutTextXY3D(159, 140, "playing with any key or quit", 1, 2);
    OutTextXY3D(159, 155, "by pressing ESC once again.", 1, 2);
    info_page_wait();

    /* Seite 3 */
    explain_page_begin();
    SetTextJustify(GFX_CENTER, GFX_CENTER);
    OutTextXY3D(158, 15, "objects you will meet in space", 1, 3);
    PutImage(2, 45, Jet);
    SetTextJustify(GFX_LEFT, GFX_CENTER);
    OutTextXY3D(60, 50, "Carriership \"TURBO BACS\".", 1, 2);
    OutTextXY3D(60, 65, "This is the ship you have to", 1, 2);
    OutTextXY3D(60, 80, "control.", 1, 2);
    PutImage(15, 120, Ufo[0]);
    OutTextXY3D(60, 105, "Unknown Flying Object (UFO).", 2, 2);
    OutTextXY3D(60, 120, "These small ships try to", 2, 2);
    OutTextXY3D(60, 135, "destroy other ships by", 2, 2);
    OutTextXY3D(60, 150, "crashing into them. More", 2, 2);
    OutTextXY3D(60, 165, "information is not available.", 2, 2);
    info_page_wait();

    /* Seite 4 */
    explain_page_begin();
    SetTextJustify(GFX_CENTER, GFX_CENTER);
    OutTextXY3D(158, 15, "objects you will meet in space", 1, 3);
    SetTextJustify(GFX_LEFT, GFX_CENTER);
    PutImage(10, 60, Ship1);
    OutTextXY3D(60, 50, "Fighter \"ACOMBA\". Has two guns", 3, 2);
    OutTextXY3D(60, 65, "which can fire oxygen", 3, 2);
    OutTextXY3D(60, 80, "rockets. Ship is considered", 3, 2);
    OutTextXY3D(60, 95, "to be very hazardous.", 3, 2);
    PutImage(12, 115, Meteorit);
    OutTextXY3D(60, 115, "Meteorit. Large rock which", 1, 2);
    OutTextXY3D(60, 130, "can be found nearly everywhere.", 1, 2);
    OutTextXY3D(60, 145, "in space. Can destroy spaceships", 1, 2);
    OutTextXY3D(60, 160, "if they approach.", 1, 2);
    info_page_wait();

    /* Seite 5 */
    explain_page_begin();
    SetTextJustify(GFX_CENTER, GFX_CENTER);
    OutTextXY3D(158, 15, "objects you will meet in space", 1, 3);
    SetTextJustify(GFX_LEFT, GFX_CENTER);
    PutImage(10, 60, Barrel);
    OutTextXY3D(60, 50, "Barrel filled with fuel.", 2, 2);
    OutTextXY3D(60, 65, "Shooting at barrel will", 2, 2);
    OutTextXY3D(60, 80, "refill your ships fuel reserve.", 2, 2);
    PutImage(10, 115, Wuerfel[0]);
    OutTextXY3D(60, 115, "CSS (Cube Space Ship)", 3, 2);
    OutTextXY3D(60, 130, "The crews of these ships", 3, 2);
    OutTextXY3D(60, 145, "fix broken planes. Shoot", 3, 2);
    OutTextXY3D(60, 160, "at them to get a new ship.", 3, 2);
    info_page_wait();

    /* Seite 6 */
    explain_page_begin();
    SetTextJustify(GFX_CENTER, GFX_CENTER);
    OutTextXY3D(158, 15, "tips and hints for space pilots", 1, 3);
    OutTextXY3D(158, 50, "Always watch the fuel display", 2, 2);
    OutTextXY3D(158, 65, "(the long bar in the upper screen).", 2, 2);
    OutTextXY3D(158, 80, "Flying and shooting consumes fuel.", 2, 2);
    OutTextXY3D(158, 95, "If there is no fuel left at all your", 2, 2);
    OutTextXY3D(158, 110, "ship will be destroyed.", 2, 2);
    OutTextXY3D(158, 125, "Sometimes clouds of small rocks", 2, 2);
    OutTextXY3D(158, 140, "appear on screen. Try keeping away", 2, 2);
    OutTextXY3D(158, 155, "from them because shooting at those", 2, 2);
    OutTextXY3D(158, 170, "rocks has no effect.", 2, 2);
    info_page_wait();

    /* Seite 7 */
    explain_page_begin();
    SetTextJustify(GFX_CENTER, GFX_CENTER);
    OutTextXY3D(158, 15, "tips and hints for space pilots", 1, 3);
    OutTextXY3D(158, 50, "If you want to stop your ship", 3, 2);
    OutTextXY3D(158, 65, "moving you can press all keys", 3, 2);
    OutTextXY3D(158, 80, "except those you need for ship", 3, 2);
    OutTextXY3D(158, 95, "control.", 3, 2);
    OutTextXY3D(158, 110, "You can toggle the sound on or", 3, 2);
    OutTextXY3D(158, 125, "off using the setup option.", 3, 2);
    OutTextXY3D(158, 140, "Slowing down the game is possible", 3, 2);
    OutTextXY3D(158, 155, "by entering a number at the speed", 3, 2);
    OutTextXY3D(158, 170, "option of the setup menu.", 3, 2);
    OutTextXY3D(158, 185, "A big number means a slow game.", 3, 2);
    info_page_wait();
}

static void FinishGame(void) {
    int i;
    Bar(0, 0, 319, 199);
    SetTextJustify(GFX_LEFT, GFX_LEFT);
    SetUserCharSize(13, 16, 16, 16);
    SetColor(2);
    OutTextXY(4, 30, "Thanks for playing");
    SetUserCharSize(16, 16, 16, 16);
    for (i = 1; i <= 5; i++) {
        SetColor(Succ(i % 3));
        OutTextXY(45 + i, 75 + i, "Spaceflight");
        if (Keypressed()) goto done;
    }
    for (i = 1; i <= 5; i++) {
        SetColor(Succ(i % 3));
        OutTextXY(65 + i, 125 + i, "Spaceflight");
        if (Keypressed()) goto done;
    }
    NoSound();
    SetUserCharSize(10, 16, 16, 16);
    OutTextXY(4, 177, "press any key to continue...");
done:
    sx = 160; sy = 100;
    InitPageStars();
    wait_stars();
    Finished = 1;
    NoSound();
    SaveHighScores();
}

/* ---------------------------------------------------------------------- */
/* Konfiguration / main                                                   */
/* ---------------------------------------------------------------------- */
static void parse_args(int argc, char **argv, plat_config *cfg) {
    int k;
    for (k = 1; k < argc; k++) {
        char *s = argv[k];
        if (!strcmp(s, "--windowed")) cfg->fullscreen = 0;
        else if (!strcmp(s, "--fullscreen")) cfg->fullscreen = 1;
        else if (!strcmp(s, "--width")  && k + 1 < argc) cfg->width  = atoi(argv[++k]);
        else if (!strcmp(s, "--height") && k + 1 < argc) cfg->height = atoi(argv[++k]);
        else if (!strcmp(s, "--aspect85")) cfg->aspect43 = 0;
        else if (!strcmp(s, "--aspect43")) cfg->aspect43 = 1;
        else if (!strcmp(s, "--integer")) cfg->integer_scale = 1;
        else if (!strcmp(s, "--novsync")) cfg->vsync = 0;
        else if (!strcmp(s, "--noeffects")) { cfg->fx_scanline = cfg->fx_glow = cfg->fx_vignette = 0; }
        else if (!strcmp(s, "--scanline") && k + 1 < argc) cfg->fx_scanline = (float)atof(argv[++k]);
        else if (!strcmp(s, "--glow")     && k + 1 < argc) cfg->fx_glow     = (float)atof(argv[++k]);
        else if (!strcmp(s, "--vignette") && k + 1 < argc) cfg->fx_vignette = (float)atof(argv[++k]);
        else if (!strcmp(s, "--fps")      && k + 1 < argc) g_base_fps       = atoi(argv[++k]);
        else if (!strcmp(s, "--demo")) { g_demo = 1; cfg->fullscreen = 0; }
        else if (!strcmp(s, "--demosetup")) { g_demo = 2; cfg->fullscreen = 0; }
        else if (!strcmp(s, "--demoexplain")) { g_demo = 3; cfg->fullscreen = 0; }
    }
    if (g_base_fps < 20) g_base_fps = 20;
    if (g_base_fps > 240) g_base_fps = 240;
}

int main(int argc, char **argv) {
    memset(&g_cfg, 0, sizeof(g_cfg));
    g_cfg.fullscreen = 1;
    g_cfg.width = 0; g_cfg.height = 0;   /* native Desktop-Aufloesung */
    g_cfg.aspect43 = 1;
    g_cfg.integer_scale = 0;
    g_cfg.vsync = 1;
    g_cfg.fx_scanline = 0.20f;
    g_cfg.fx_glow = 0.25f;
    g_cfg.fx_vignette = 0.35f;

    LoadHighScores();                    /* setzt u.a. gespeicherte Anzeige-Prefs */
    parse_args(argc, argv, &g_cfg);      /* Kommandozeile ueberschreibt */

    srand((unsigned)time(NULL) ^ (unsigned)(plat_time() * 1000.0));

    if (plat_init(&g_cfg) != 0) {
        MessageBoxA(NULL, "Direct3D 11 konnte nicht initialisiert werden.",
                    "Spaceflight", MB_ICONERROR);
        return 1;
    }
    install_font();

    if (g_demo) SoundOn = 1;          /* Audiopfad im Selbsttest ausueben */
    SetScreenMode();
    TitleScreen();

    Finished = 0;
    while (g_running && !Finished) {
        switch (MainMenu()) {
            case 1: HiScoreScreen(); break;
            case 2: StartGame();     break;
            case 3: ExplainGame();   break;
            case 4: Setup();         break;
            case 5: FinishGame();    break;
        }
    }

    plat_shutdown();
    return 0;
}
