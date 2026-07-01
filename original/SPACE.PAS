PROGRAM SpaceFlight;

{$M 65520,0,655360}
{$I-}

USES Crt,Graph,Dos,SpacData;


PROCEDURE InitGame;
BEGIN
  FOR i := 1 TO MaxStar DO
  BEGIN
    StarX [i] := Random (2560);
    OldStarX [i] := 2560;
    StarY [i] := Random (182) + 18;
    StarMove [i] := Random (MaxSpeed) + MinSpeed;
  END;
  FOR i := 1 TO MaxMeteoritStar DO
  BEGIN
    OldMStarX [i] := 2000;
    OldMStarY [i] := 1000;
  END;
  x := MinX;
  y := 140;
  ShipMoveY := 0;
  ShipMoveX := 0;
  Taste := 0;
  StarMaxSpeed := MaxSpeed;
  Shoot := False;
  ShootCount := 0;
  Meteor := False;
  FOR i := 1 TO MaxUfoAnz DO
    UfoOnScreen [i] := False;
  BarrelOnScreen := False;
  EnemyShootOnScreen := False;
  WuerfelOnScreen := False;
  BrockenOnScreen := False;
  Score := 0;
  Fuel := 312;
  MeteorDestroyed := False;
  FuelCount := MaxFuelCount;
  UfoRandom := UfoRandomAnf;
  WuerfelRandom := WuerfelRandomAnf;
  BrockenRandom := BrockenRandomAnf;
  EnemyRandom := EnemyRandomAnf;
  EnemyShootRandom := EnemyShootRandomAnf;
  MaxBrockenAnz := MaxBrockenAnzAnf;
END;


PROCEDURE PrintLives;
BEGIN
  PutImage (180 + Lives * 19,5,Leben,CopyPut);
END;


PROCEDURE DelLive;
BEGIN
  PutImage (180 + Succ (Lives) * 19,5,LebenLoesch,CopyPut);
END;


PROCEDURE PrintScore;

VAR
  hstr            :         String [7];

BEGIN
  Bar (50,3,112,11);
  SetColor (3);
  Str (Score:7,hstr);
  OutTextXY (50,3,hstr);
  IF Random (SchwierigerRandom) = 0 THEN
  BEGIN
    WuerfelRandom := WuerfelRandom + Random (47);
    IF WuerfelRandom > 10000 THEN WuerfelRandom := 10000;
    BrockenRandom := BrockenRandom - Random (15);
    IF BrockenRandom < 39 THEN BrockenRandom := 39;
    EnemyRandom := EnemyRandom - Random (7);
    IF EnemyRandom < 31 THEN EnemyRandom := 31;
    EnemyShootRandom := EnemyShootRandom - Random (4);
    IF EnemyShootRandom < 7 THEN EnemyShootRandom := 7;
    IF Random (16) = 0 THEN MaxBrockenAnz := Succ (MaxBrockenAnz);
    IF MaxBrockenAnz > MaxMaxBrockenAnz THEN MaxBrockenAnz := MaxMaxBrockenAnz;
  END;
END;


PROCEDURE PrintFuel;
BEGIN
  SetColor (0);
  Line (Fuel + 4,13,Fuel + 4,15);
END;


PROCEDURE InitScreen;
BEGIN
  SetScreenMode;
  SetColor (1);
  MoveTo (0,0);
  LineTo (319,0);
  LineTo (319,17);
  LineTo (0,17);
  LineTo (0,0);
  SetColor (2);
  Line (4,13,315,13);
  Line (4,14,315,14);
  Line (4,15,315,15);
  SetColor (3);
  Line (4,13,50,13);
  Line (4,14,50,14);
  Line (4,15,50,15);
  SetColor (2);
  OutTextXY (3,3,'Score:');
  PrintScore;
  OutTextXY (140,3,'Lives:');
  FOR Lives := 1 TO 6 DO
    PrintLives;
END;


PROCEDURE GameEnd;

VAR
  PufferX,PufferY         :       Array [0..10000] OF Integer;
  i                       :       Word;

BEGIN
  SetColor (1 );
  FOR i := 169 DownTo 0 DO
  BEGIN
    MoveTo (i,Round (i / 1.6));
    LineTo (i,199 - Round (i / 1.6));
    LineTo (319 - i,199 - Round (i / 1.6));
    LineTo (319 - i,Round (i / 1.6));
    LineTo (i,Round (i / 1.6));
  END;
  GotHiScore;
  While Keypressed DO a := ReadKey;
END;


PROCEDURE MoveMeteoritStars;

VAR
  i          :        Word;

BEGIN
  FOR i := 1 TO MaxMeteoritStar DO
  BEGIN
    IF MStarX [i] <> 0 THEN
    BEGIN
      PutPixel (OldMStarX [i] SHR 2,OldMStarY [i] SHR 2,0);
      IF (GetPixel (MStarX [i] SHR 2,MStarY [i] SHR 2) = 0) AND
      (MStarY [i] > 68) THEN
      BEGIN
        PutPixel (MStarX [i] SHR 2,MStarY [i] SHR 2,3);
        OldMStarX [i] := MStarX [i];
        OldMStarY [i] := MStarY [i];
      END ELSE OldMStarX [i] := 1280;
      MStarX [i] := MStarX [i] + MStarMoveX [i];
      MStarY [i] := MStarY [i] + MStarMoveY [i];
      IF (MStarX [i] < 4) OR (MStarX [i] > 1280) OR
      (MStarY [i] < 68) OR (MStarY [i] > 800) THEN
      BEGIN
        PutPixel (OldMStarX [i] SHR 2,OldMStarY [i] SHR 2,0);
        MStarX [i] := 0;
      END;
    END;
  END;
  MeteorDestroyedCount := Pred (MeteorDestroyedCount);
  IF MeteorDestroyedCount = 0 THEN
  BEGIN
    MeteorDestroyed := False;
    FOR i := 1 TO MaxMeteoritStar DO
    BEGIN
      PutPixel (OldMStarX [i] SHR 2,OldMStarY [i] SHR 2,0);
    END;
  END;
END;


PROCEDURE MeteoritDestroyed;

VAR
  i           :      Byte;

BEGIN
  IF SoundOn THEN Sound (100);
  FOR i := 1 TO MaxMeteoritStar DO
  BEGIN
    MStarX [i] := (mx + 12) SHL 2;
    MStarY [i] := (my + 12) SHL 2;
    MStarMoveX [i] := Random (33) - 16;
    MStarMoveY [i] := Random (33) - 16;
  END;
  IF SoundOn THEN Sound (200);
  MeteorDestroyed := True;
  MeteorDestroyedCount := 100;
  mx := 0;
  IF SoundOn THEN Sound (100);
  MoveMeteoritStars;
  NoSound;
END;


PROCEDURE ShootAtMeteor;

VAR
  ShootFlag         :      Boolean;

BEGIN
  ShootFlag := False;
  IF (ShootY [j] > my) AND (ShootY [j] < my + 24) THEN
  BEGIN
    FOR p := ShootX [j] TO ShootX [j] + ShootAdd DO
    IF (p < mx + 24) AND (p > mx) THEN
    BEGIN
      ShootFlag := True;
      p := ShootX [j] + ShootAdd;
      SetColor (0);
      Line (ShootX [j],ShootY [j],ShootX [j] + ShootAdd,ShootY [j]);
      ShootX [j] := 319;
    END;
  END;
  IF ShootFlag THEN
  BEGIN
    Meteor := False;
    PutImage (mx,my,MeteoritLoesch,CopyPut);
    Score := Score + (300 - (mx - x)) DIV 2;
    PrintScore;
    MeteorDestroyed := True;
    MeteoritDestroyed;
  END;
END;


PROCEDURE ShootAtBarrel;

VAR
  ShootFlag         :      Boolean;

BEGIN
  ShootFlag := False;
  IF (ShootY [j] > by) AND (ShootY [j] < by + 24) THEN
  BEGIN
    FOR p := ShootX [j] TO ShootX [j] + ShootAdd DO
    IF (p < bx + 13) AND (p > bx) THEN
    BEGIN
      ShootFlag := True;
      p := ShootX [j] + ShootAdd;
      SetColor (0);
      Line (ShootX [j],ShootY [j],ShootX [j] + ShootAdd,ShootY [j]);
      ShootX [j] := 319;
    END;
  END;
  IF ShootFlag THEN
  BEGIN
    IF SoundOn THEN Sound (300);
    BarrelOnScreen := False;
    Bar (bx,by,bx + 24,by + 13);
    Score := Score + (300 - (bx - x)) * 2;
    PrintScore;
    Fuel := 312;
    SetColor (2);
    Line (4,13,315,13);
    Line (4,14,315,14);
    Line (4,15,315,15);
    SetColor (3);
    Line (4,13,50,13);
    Line (4,14,50,14);
    Line (4,15,50,15);
    NoSound;
  END;
END;


PROCEDURE ShootAtWuerfel;

VAR
  ShootFlag         :      Boolean;

BEGIN
  ShootFlag := False;
  IF (ShootY [j] > wy) AND (ShootY [j] < wy + 22) THEN
  BEGIN
    FOR p := ShootX [j] TO ShootX [j] + ShootAdd DO
    IF (p < wx + 28) AND (p > wx) THEN
    BEGIN
      ShootFlag := True;
      p := ShootX [j] + ShootAdd;
      SetColor (0);
      Line (ShootX [j],ShootY [j],ShootX [j] + ShootAdd,ShootY [j]);
      ShootX [j] := 319;
    END;
  END;
  IF ShootFlag THEN
  BEGIN
    IF Lives < 6 THEN
    BEGIN
      IF SoundOn THEN Sound (900);
      WuerfelOnScreen := False;
      Bar (wx,wy,wx + 28,wy + 22);
      Score := Score + (300 - (wx - x)) * 2;
      PrintScore;
      IF SoundOn THEN Sound (450);
      Lives := Succ (Lives);
      IF SoundOn THEN Sound (225);
      PrintLives;
      NoSound;
    END;
  END;
END;


PROCEDURE ShootAtUfo;

VAR
  ShootFlag         :      Boolean;

BEGIN
  FOR i := 1 TO MaxUfoAnz DO
  IF UfoOnScreen [i] THEN
  BEGIN
    ShootFlag := False;
    IF (ShootY [j] > uy [i] - 3) AND (ShootY [j] < uy [i] + 11) THEN
    BEGIN
      FOR p := ShootX [j] TO ShootX [j] + ShootAdd DO
      IF (p < ux [i] + 31) AND (p > ux [i]) THEN
      BEGIN
        ShootFlag := True;
        p := ShootX [j] + ShootAdd;
        SetColor (0);
        Line (ShootX [j],ShootY [j],ShootX [j] + ShootAdd,ShootY [j]);
        ShootX [j] := 319;
      END;
    END;
    IF ShootFlag THEN
    BEGIN
      IF SoundOn THEN Sound (35);
      UfoOnScreen [i] := False;
      PutImage (ux [i],uy [i],UfoLoesch,CopyPut);
      Score := Score + (300 - (ux [i] - x));
      PrintScore;
      NoSound;
    END;
  END;
END;


PROCEDURE ShootAtEnemy;

VAR
  ShootFlag         :      Boolean;

BEGIN
  ShootFlag := False;
  IF (ShootY [j] > ey) AND (ShootY [j] < ey + 23) THEN
  BEGIN
    FOR p := ShootX [j] TO ShootX [j] + ShootAdd DO
    IF (p < ex + 39) AND (p > ex) THEN
    BEGIN
      ShootFlag := True;
      p := ShootX [j] + ShootAdd;
      SetColor (0);
      Line (ShootX [j],ShootY [j],ShootX [j] + ShootAdd,ShootY [j]);
      ShootX [j] := 319;
    END;
  END;
  IF ShootFlag THEN
  BEGIN
    IF SoundOn THEN Sound (100);
    EnemyOnScreen := False;
    Bar (ex,ey,ex + 39,ey + 23);
    Score := Score + (300 - (mx - x)) * 2;
    PrintScore;
    NoSound;
  END;
END;


PROCEDURE MoveShoot;

VAR
  k               :        Byte;
  ShootGone       :        Boolean;

BEGIN
  NoSound;
  ShootGone := False;
  FOR j := 1 TO ShootCount DO
  BEGIN
    IF Meteor THEN ShootAtMeteor;
    IF BarrelOnScreen THEN ShootAtBarrel;
    IF WuerfelOnScreen THEN ShootAtWuerfel;
    ShootAtUfo;
    IF EnemyOnScreen THEN ShootAtEnemy;
    ShootX [j] := ShootX [j] + ShootAdd;
    SetColor (0);
    Line (ShootX [j] - ShootAdd,ShootY [j],ShootX [j],ShootY [j]);
    IF ShootX [j] < 319 THEN
    BEGIN
      SetColor (2);
      Line (ShootX [j],ShootY [j],ShootX [j] + ShootAdd,ShootY [j]);
    END
    ELSE
    ShootGone := True;
  END;
  IF ShootGone THEN
  BEGIN
    k := ShootCount;
    FOR j := 1 TO k DO
    BEGIN
      IF ShootX [j] > 319 THEN
      BEGIN
        ShootX [j] := ShootX [ShootCount];
        ShootY [j] := ShootY [ShootCount];
        ShootCount := Pred (ShootCount);
      END;
    END;
  END;
  IF ShootCount = 0 THEN Shoot := False;
END;


PROCEDURE ControlKeys;
BEGIN
  Taste := Key;
  CASE Taste OF
 72 : BEGIN
        ShipMoveY := -3;
        ShipMoveX := 0;
      END;
 80 : BEGIN
        ShipMoveY := 3;
        ShipMoveX := 0;
      END;
 75 : BEGIN
        ShipMoveX := -3;
        ShipMoveY := 0;
      END;
 77 : BEGIN
        ShipMoveX := 3;
        ShipMoveY := 0;
      END;
 71 : BEGIN
        ShipMoveX := -3;
        ShipMoveY := -3;
      END;
 73 : BEGIN
        ShipMoveX := 3;
        ShipMoveY := -3;
      END;
 79 : BEGIN
        ShipMoveX := -3;
        ShipMoveY := 3;
      END;
 81 : BEGIN
        ShipMoveX := 3;
        ShipMoveY := 3;
      END;
 57 : BEGIN
        IF ShootCount < MaxShootAnz THEN
        BEGIN
          IF SoundOn THEN Sound (60);
          Shoot := True;
          Fuel := Pred (Fuel);
          PrintFuel;
          ShootCount := Succ (ShootCount);
          ShootX [ShootCount] := x + 58;
          ShootY [ShootCount] := y + 15;
        END;
      END;
  1 : BEGIN
        GetImage (0,0,319,199,Puffer);
        SetTextStyle (TriplexFont,HorizDir,0);
        SetTextJustify (LeftText,LeftText);
        SetUserCharSize (10,16,12,16);
        OutTextXY3D (60,70,'pausing . . .',1,3);
        OutTextXY3D (10,120,'press ESC to finish game or',2,3);
        OutTextXY3D (10,150,'any other key to continue !',3,3);
        ClearInput;
        a := ReadKey;
        IF a <> Chr (27) THEN Taste := 0;
        SetScreenMode;
        PutImage (0,0,Puffer,CopyPut);
      END
      ELSE
      BEGIN
        ShipMoveX := 0;
        ShipMoveY := 0;
      END;
  END;
END;


PROCEDURE MoveStars;

VAR
  i          :        Word;

BEGIN
  FOR i := 1 TO MaxStar DO
  BEGIN
    PutPixel (OldStarX [i] SHR 3,StarY [i],0);
    IF GetPixel (StarX [i] SHR 3,StarY [i]) = 0 THEN
    BEGIN
      PutPixel (StarX [i] SHR 3,StarY [i],3);
      OldStarX [i] := StarX [i];
    END ELSE OldStarX [i] := 2560;
    StarX [i] := StarX [i] - StarMove [i];
    IF (StarX [i] < 0) THEN
    BEGIN
      PutPixel ((StarX [i] + StarMove [i]) SHR 3,StarY [i],0);
      StarX [i] := Random (120) + 2440;
      StarY [i] := Random (182) + 18;
      StarMove [i] := Random (StarMaxSpeed) + MinSpeed;
    END;
  END;
END;


PROCEDURE MoveShip;
BEGIN
  IF ShipMoveY <> 0 THEN
  BEGIN
    IF (y < MinY) AND (ShipMoveY < 0) THEN ShipMoveY := 0;
    IF (y > MaxY) AND (ShipMoveY > 0) THEN ShipMoveY := 0;
    y := y + ShipMoveY;
  END;
  IF ShipMoveX <> 0 THEN
  BEGIN
    IF (x < MinX) AND (ShipMoveX < 0) THEN ShipMoveX := 0;
    IF (x > MaxX) AND (ShipMoveX > 0) THEN ShipMoveX := 0;
    x := x + ShipMoveX;
    StarMaxSpeed := StarMaxSpeed;
  END;
  PutImage (x,y,Jet,CopyPut);
END;


PROCEDURE MakeMeteor;
BEGIN
  IF Random (MeteorRandom) = 0 THEN
  BEGIN
    Meteor := True;
    mx := 296;
    my := Random (162) + 18;
    mxadd := Succ (Random (3));
    PutImage (mx,my,Meteorit,CopyPut);
  END;
END;


PROCEDURE InitJetStars;

VAR
  i           :      Byte;

BEGIN
  FOR i := 1 TO MaxJetStar DO
  BEGIN
    JStarX [i] := (sx + Random (58) - 25) SHL 2;
    JStarY [i] := (sy + Random (30) - 15) SHL 2;
    JStarMoveX [i] := Random (45) - 22;
    JStarMoveY [i] := Random (45) - 22;
    OldJStarX [i] := 1290;
  END;
END;


PROCEDURE MoveShipDestroyedStars;

VAR
  i          :        Word;

BEGIN
  FOR i := 1 TO MaxJetStar DO
  BEGIN
    PutPixel (x + Random (58),y + Random (30),0);
    PutPixel (OldJStarX [i] SHR 2,OldJStarY [i] SHR 2,0);
    IF (GetPixel (JStarX [i] SHR 2,JStarY [i] SHR 2) = 0) THEN
    BEGIN
      PutPixel (JStarX [i] SHR 2,JStarY [i] SHR 2,Succ (Random (3)));
      OldJStarX [i] := JStarX [i];
      OldJStarY [i] := JStarY [i];
    END ELSE OldJStarX [i] := 1290;
    JStarX [i] := JStarX [i] + JStarMoveX [i];
    JStarY [i] := JStarY [i] + JStarMoveY [i];
    IF (JStarX [i] < 4) OR (JStarX [i] > 1280) OR
    (JStarY [i] < 76) OR (JStarY [i] > 800) THEN
    BEGIN
      PutPixel (OldJStarX [i] SHR 2,OldJStarY [i] SHR 2,0);
      JStarX [i] := sx SHL 2;
      JStarY [i] := sy SHL 2;
      JStarMoveX [i] := (Random (45) - 22) * 2;
      JStarMoveY [i] := (Random (45) - 22) * 2;
      IF JStarMoveX [i] = 0 THEN JStarMoveX [i] := 9;
    END;
  END;
END;


PROCEDURE ShipLost;

VAR
  i         :         Byte;

BEGIN
  sx := x + 25;
  sy := y + 15;
  InitJetStars;
  FOR i := 1 TO 25 DO
  BEGIN
    IF SoundOn THEN Sound (50 + Random (50) - i * 2);
    PutPixel (x + Random (58),y + Random (30),0);
    MoveShipDestroyedStars;
    MoveStars;
    PutPixel (x + Random (58),y + Random (30),0);
    IF SoundOn THEN Sound (50 + Random (50) - i * 2);
  END;
  FOR i := 1 TO MaxJetStar DO
    PutPixel (OldJStarX [i] SHR 2,OldJStarY [i] SHR 2,0);
  NoSound;
  Lives := Pred (Lives);
  DelLive;
  IF Lives = 0 THEN GameEnd;
END;


PROCEDURE ControlCrash;

VAR
  i           :      Byte;

BEGIN
  IF Meteor THEN
  BEGIN
    IF (mx + 16 > x) AND (mx - 54 < x) THEN
    IF (my + 19 > y) AND (my - 24 < y) THEN
    BEGIN
      ShipLost;
      PutImage (mx,my,MeteoritLoesch,CopyPut);
      MeteoritDestroyed;
      PutImage (x,y,Jet,CopyPut);
    END;
  END;
  FOR i := 1 TO MaxUfoAnz DO
  BEGIN
    IF UfoOnScreen [i] THEN
    BEGIN
      IF (ux [i] + 22 > x) AND (ux [i] - 51 < x) THEN
      IF (uy [i] + 10 > y) AND (uy [i] - 25 < y) THEN
      BEGIN
        ShipLost;
        PutImage (ux [i],uy [i],UfoLoesch,CopyPut);
        UfoOnScreen [i] := False;
        PutImage (x,y,Jet,CopyPut);
      END;
    END;
  END;
  IF BrockenOnScreen THEN
  BEGIN
    FOR i := 1 TO BrockenAnz DO
    BEGIN
      IF (BrockenX [i] + 18 > x) AND (BrockenX [i] - 51 < x) THEN
      IF (BrockenY [i] + 10 > y) AND (BrockenY [i] - 25 < y) THEN
      BEGIN
        ShipLost;
        Bar (BrockenX [i],BrockenY [i],BrockenX [i] + 29,BrockenY [i] + 13);
        BrockenX [i] := - 4;
        BrockenY [i] := 400;
        PutImage (x,y,Jet,CopyPut);
      END;
    END;
  END;
END;


PROCEDURE MoveMeteor;
BEGIN
  mx := mx - mxadd;
  IF mx > 0 THEN PutImage (mx,my,Meteorit,CopyPut)
  ELSE
  BEGIN
    Meteor := False;
    mx := mx + mxadd;
    PutImage (mx,my,MeteoritLoesch,CopyPut);
  END;
END;


PROCEDURE MakeUfo;
BEGIN
  FOR i := 1 TO MaxUfoAnz DO
  BEGIN
    IF UfoOnScreen [i] = False THEN
    IF Random (UfoRandom) = 0 THEN
    BEGIN
      UfoOnScreen [i] := True;
      ux [i] := 300;
      uy [i] := Random (162) + 18;
      uxadd [i] := - Succ (Random (5));
      uyadd [i] := Random (3);
      UfoCount := 0;
      Ufonr := 0;
      PutImage (ux [i],uy [i],Ufo [Ufonr],CopyPut);
    END;
  END;
END;


PROCEDURE MoveUfo;

VAR
  i        :     Byte;

BEGIN
  UfoCount := Succ (UfoCount);
  IF UfoCount = 10 THEN UfoCount := 0;
  UfoNr := UfoCount SHR 1;
  FOR i := 1 TO MaxUfoAnz DO
  BEGIN
    IF UfoOnScreen [i] THEN
    BEGIN
      ux [i] := ux [i] + uxadd [i];
      IF ux [i] > 0 THEN PutImage (ux [i],uy [i],Ufo [Ufonr],CopyPut)
      ELSE
      BEGIN
        UfoOnScreen [i] := False;
        ux [i] := ux [i] - uxadd [i];
        PutImage (ux [i],uy [i],UfoLoesch,CopyPut);
      END;
    END;
    IF y > uy [i] THEN uy [i] := uy [i] + uyadd [i] ELSE
    IF y < uy [i] THEN uy [i] := uy [i] - uyadd [i];
  END;
END;


PROCEDURE MakeBrocken;
BEGIN
  BrockenOnScreen := True;
  BrockenAnz := Succ (Random (MaxBrockenAnz));
  FOR i := 1 TO BrockenAnz DO
  BEGIN
    BrockenX [i] := 300 + Random (60);
    BrockenY [i] := Random (162) + 18;
    BrockenmoveX [i] := - Succ (Random (4));
    Brockennr [i] := Random (4);
    Brockenweg [i] := False;
    PutImage (BrockenX [i],BrockenY [i],Brocken [BrockenNr [i]],CopyPut);
  END;
  BrockenCount := BrockenAnz;
END;


PROCEDURE MoveBrocken;

VAR
  i        :     Byte;

BEGIN
  FOR i := 1 TO BrockenAnz DO
  BEGIN
    BrockenX [i] := BrockenX [i] + BrockenmoveX [i];
    IF BrockenX [i] > 0 THEN PutImage (BrockenX [i],BrockenY [i],Brocken [BrockenNr [i]],CopyPut)
    ELSE
    BEGIN
      IF Brockenweg [i] = False THEN
      BEGIN
        Bar (BrockenX [i],BrockenY [i],BrockenX [i] + 20,BrockenY [i] + 13);
        BrockenCount := Pred (BrockenCount);
        IF BrockenCount = 0 THEN BrockenOnScreen := False;
        Brockenweg [i] := True;
      END;
    END;
  END;
END;


PROCEDURE ShootAtSpaceShip;

VAR
  ShootFlag         :      Boolean;

BEGIN
  ShootFlag := False;
  IF (esy2 > y) AND (esy1 < y + 29) THEN
  BEGIN
    FOR p := esx TO esx + exadd DO
    IF (p < x + 39) AND (p > x) THEN
    BEGIN
      ShootFlag := True;
      p := esx + exadd;
      SetColor (0);
      Line (esx + exadd,esy1,esx,esy1);
      Line (esx + exadd,esy2,esx,esy2);
      esx := -2;
    END;
  END;
  IF ShootFlag THEN
  BEGIN
    IF SoundOn THEN Sound (70);
    ShipLost;
    NoSound;
  END;
END;


PROCEDURE EnemyShoots;
BEGIN
  esy1 := ey + 5;
  esy2 := ey + 17;
  exadd := EnemyShootAdd + Random (3);
  esx := ex;
  SetColor (2);
  Line (esx + exadd,esy1,esx,esy1);
  Line (esx + exadd,esy2,esx,esy2);
  EnemyShootOnScreen := True;
END;


PROCEDURE MoveEnemyShoot;
BEGIN
  SetColor (0);
  Line (esx + exadd,esy1,esx,esy1);
  Line (esx + exadd,esy2,esx,esy2);
  esx := esx - exadd;
  IF esx > 0 THEN
  BEGIN
    SetColor (Succ (Random (3)));
    Line (esx + exadd,esy1,esx,esy1);
    Line (esx + exadd,esy2,esx,esy2);
    ShootAtSpaceShip;
  END
  ELSE EnemyShootOnScreen := False;
END;


PROCEDURE MakeEnemy;
BEGIN
  EnemyOnScreen := True;
  ex := 290;
  ey := Random (162) + 18;
  exmove := - Succ (Random (5));
  eymove := Random (3);
  PutImage (ex,ey,Ship1,CopyPut);
END;


PROCEDURE MoveEnemy;
BEGIN
  IF ex > x THEN ex := ex + exmove ELSE ex := ex - 1;
  IF ex > 0 THEN PutImage (ex,ey,Ship1,CopyPut)
  ELSE
  BEGIN
    EnemyOnScreen := False;
    ex := ex - exmove;
    Bar (ex,ey,ex + 39,ey + 23);
  END;
  IF y > ey THEN ey := ey + eymove ELSE
  IF y < ey THEN ey := ey - eymove;
  IF (EnemyShootOnScreen = False) THEN
  IF Random (EnemyShootRandom) = 0 THEN EnemyShoots;
END;


PROCEDURE InitBarrel;
BEGIN
  bx := 310;
  by := Random (150) + 26;
  bmove := Succ (Random (5));
  PutImage (bx,by,Barrel,CopyPut);
  BarrelOnScreen := True;
END;


PROCEDURE MoveBarrel;
BEGIN
  bx := bx - bmove;
  IF bx < 0 THEN
  BEGIN
    BarrelOnScreen := False;
    Bar (bx,by,bx + 24,by + 13);
  END ELSE PutImage (bx,by,Barrel,CopyPut);
END;


PROCEDURE MakeWuerfel;
BEGIN
  wx := 300;
  wy := Random (150) + 26;
  wxmove := Succ (Random (5));
  WuerfelCount := Random (32);
  PutImage (wx,wy,Wuerfel [WuerfelCount SHR 3],CopyPut);
  WuerfelOnScreen := True;
END;


PROCEDURE MoveWuerfel;
BEGIN
  wx := wx - wxmove;
  WuerfelCount := Succ (WuerfelCount);
  IF WuerfelCount = MaxWuerfelCount THEN WuerfelCount := 0;
  IF wx < 0 THEN
  BEGIN
    WuerfelOnScreen := False;
    Bar (wx,wy,wx + 28,wy + 22);
  END ELSE PutImage (wx,wy,Wuerfel [WuerfelCount SHR 3],CopyPut);
END;


PROCEDURE ConsumeFuel;
BEGIN
  FuelCount := Pred (FuelCount);
  IF FuelCount = 0 THEN
  BEGIN
    Fuel := Pred (Fuel);
    PrintFuel;
    IF Fuel <= 0 THEN
    BEGIN
      ShipLost;
      Fuel := 312;
      SetColor (2);
      Line (4,13,315,13);
      Line (4,14,315,14);
      Line (4,15,315,15);
      SetColor (3);
      Line (4,13,50,13);
      Line (4,14,50,14);
      Line (4,15,50,15);
    END;
    FuelCount := MaxFuelCount;
  END;
END;


PROCEDURE StartGame;
BEGIN
  SetScreenMode;
  InitGame;
  InitScreen;
  PutImage (x,y,Jet,CopyPut);
  Repeat
    MoveStars;
    IF Keypressed THEN ControlKeys;
    IF (ShipMoveY <> 0) OR (ShipMoveX <> 0) THEN MoveShip
    ELSE PutImage (x,y,Jet,CopyPut);
    IF Shoot THEN MoveShoot;
    IF (Meteor = False) AND (MeteorDestroyed = False) THEN MakeMeteor;
    IF Meteor THEN MoveMeteor
    ELSE IF MeteorDestroyed THEN MoveMeteoritStars;
    IF BarrelOnScreen THEN MoveBarrel ELSE
    IF Random (BarrelRandom) = 0 THEN InitBarrel;
    IF EnemyOnScreen THEN MoveEnemy ELSE
    IF Random (EnemyRandom) = 0 THEN MakeEnemy;
    IF WuerfelOnScreen THEN MoveWuerfel ELSE
    IF Random (WuerfelRandom) = 0 THEN MakeWuerfel;
    IF BrockenOnScreen THEN MoveBrocken ELSE
    IF Random (BrockenRandom) = 0 THEN MakeBrocken;
    IF EnemyShootOnScreen THEN MoveEnemyShoot;
    MoveUfo;
    MakeUfo;
    ConsumeFuel;
    ControlCrash;
    Delay (SpeedDelay);
  UNTIL (Taste = 1) OR (Lives = 0);
  ClearInput;
END;


BEGIN;
  Repeat
    CASE MainMenu OF
  1 : HiScoreScreen;
  2 : StartGame;
  3 : ExplainGame;
  4 : Setup;
  5 : FinishGame;
    END;
  UNTIL Finished = True;
  RestoreCRTMode;
END.