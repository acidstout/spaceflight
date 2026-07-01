UNIT SpacData;



INTERFACE

USES Dos,Crt,Graph;


CONST
  MaxStar               =            30;
  MaxMeteoritStar       =            40;
  MaxPageStar           =           110;
  MaxJetStar            =            35;
  MaxFuelCount          =            30;
  MaxSpeed              =           100;
  MinSpeed              =             4;
  ShipX                 =            30;
  ShipY                 =            66;
  MinY                  =            21;
  MaxY                  =           172;
  MinX                  =             4;
  MaxX                  =           240;
  BlitzDauer            =             3;
  ShootAdd              =             8;
  EnemyShootAdd         =             6;
  MeteorRandom          =           100;
  UfoRandomAnf          =           200;
  BarrelRandom          =           956;
  WuerfelRandomAnf      =          1857;
  BrockenRandomAnf      =           241;
  EnemyRandomAnf        =           109;
  EnemyShootRandomAnf   =            35;
  MaxWuerfelCount       =            32;
  MaxUfoAnz             =             5;
  MaxBrockenAnzAnf      =             5;
  MaxMaxBrockenAnz      =            20;
  MaxShootAnz           =             7;
  SchwierigerRandom     =            11;


TYPE
  ScreenType            =           Array [0..16400] OF Byte;

VAR
  mx,my,sx,sy,ex,ey,
  bx,by,bmove,Fuel,
  esx,esy1,esy2,
  exadd,
  wx,wy,wxmove,
  ShootCount,
  exmove,eymove,
  ShipMoveY,ShipMoveX,
  GraphDriver,GraphMode   :          Integer;
  UfoRandom,
  WuerfelRandom,
  BrockenRandom,
  EnemyRandom,
  EnemyShootRandom,
  MaxBrockenAnz,
  Taste,StarMaxSpeed,
  MeteorDestroyedCount,
  OldTaste,p,
  WuerfelCount,
  BrockenCount,
  BrockenAnz,
  SpeedDelay,
  FuelCount,UfoCount,
  x,y,i,j                 :          Word;
  Lives,
  mxadd,Ufonr             :          Byte;
  a                       :          Char;
  Nameein                 :          String [12];
  Datei                   :          Text;
  ScreenDatei             :          File OF ScreenType;
  Score                   :          LongInt;
  MenuScreenLoaded,
  Meteor,MeteorDestroyed,
  Finished,BarrelOnScreen,
  EnemyOnScreen,
  BrockenOnScreen,
  WuerfelOnScreen,
  SoundOn,
  EnemyShootOnScreen,
  Shoot                   :          Boolean;
  OldStarX,
  StarX,StarY             :          Array [1..MaxStar] OF Integer;
  StarMove                :          Array [1..MaxStar] OF Byte;
  MStarX,MStarY,
  MStarMoveX,MStarMoveY,
  OldMStarX,OldMStarY     :          Array [1..MaxMeteoritStar] OF Integer;
  SStarX,SStarY,
  SStarMoveX,SStarMoveY,
  OldSStarX,OldSStarY     :          Array [1..MaxPageStar] OF Integer;
  JStarX,JStarY,
  JStarMoveX,JStarMoveY,
  OldJStarX,OldJStarY     :          Array [1..MaxJetStar] OF Integer;
  ShootX,ShootY           :          Array [1..MaxShootAnz] OF Integer;
  ux,uy,uxadd,uyadd       :          Array [1..MaxUfoAnz] OF Integer;
  BrockenX,BrockenY,
  BrockenmoveX,
  BrockenmoveY            :          Array [1..MaxMaxBrockenAnz] OF Integer;
  BrockenNr               :          Array [1..MaxMaxBrockenAnz] OF Byte;
  Brockenweg              :          Array [1..MaxMaxBrockenAnz] OF Boolean;
  UfoOnScreen             :          Array [1..5] OF Boolean;
  HiScore                 :          Array [1..8] OF String [8];
  Name                    :          Array [1..8] OF String [12];
  MenuScreen              :          ScreenType;
  Puffer                  :          ScreenType;
  Reg                     :          Registers;



CONST
  Jet                   :          Array [1..487] OF Byte =
               (57,  0, 29,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  1, 80,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  3,245, 64,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,212,  0,  0,  0,
                 0,  0,  0,  0,  0,  5, 80,  0,  0,  0,255,253, 80,  0,  0,
                 0,  0,  0,  0,  0,  3,245,  0,  0,  0,255,255,245,  0,  0,
                 0,  0,  0,  0,  0,  3,255, 80,  0,  0, 63,255,255, 80,  0,
                 0,  0,  0,  0,  0,  3,255,244,  0,  0, 63,255,255,245,  0,
                 0,  0,  0,  0,  0, 15,255,216,  0,  0, 63,250,170,170, 80,
                 0,  0,  0,  0,  0, 15,245,106,168,  0, 15,168,  0, 10,170,
                 0,  0,  0,  0,  0, 21, 90,170,170,170,170,128,  0,  0, 10,
               170,128,  0,  0,  0,  0,  0, 42,170,170,170,170,170,170,170,
               165, 84,  0,  0,  0,  0,  0,  0, 42,170,191,255,255,255,214,
               170,  0,  0,  0,  0,  0,  0,  0,  0,170,191,255,255,253,106,
               128,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,255,255,214,160,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  3,255,255,253, 64,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  3,255,255,212,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0, 15,255,253, 64,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0, 15,255, 84,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0, 63,213,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0, 21, 64,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0, 11,255,128, 62,153,  0,  0,116,  6,232,209,
                20,233,254,254,128, 62,111,  0,  0,116, 15,139,193,247, 38,
               100, 19,139,216,139, 14,102);


  Meteorit           :          Array [1..207] OF Byte =
              ( 24,  0, 24,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  6,100,  0,  0,  0,  0,  0,101,101,128,  0,
                 0,  0,  1, 85,154,160,  0,  0,  0,  5,149,150,104,  0,  0,
                 0, 21, 85,105,169,  0,  0,  0, 21, 85, 86,106,  0,  0,  0,
                85,102,165, 90,128,  0,  0, 85, 85,101,102,128,  0,  0, 85,
                90,153,165,128,  0,  0, 85, 86,153,154,128,  0,  0, 85,102,
               166,170,128,  0,  0, 21, 85, 90,169,  0,  0,  0, 21,149, 90,
               166,  0,  0,  0,  5, 85,154,168,  0,  0,  0,  1, 85,106, 96,
                 0,  0,  0,  0,149, 85,128,  0,  0,  0,  0,  5,104,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,
               255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
               255,255,255,255,255,255,255,255,255,255,255,255);

  MeteoritLoesch      :          Array [1..207] OF Byte =
              ( 24,  0, 24,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0);

  Ufo                 :           Array [0..4,1..135] OF Byte =
              ((31,  0, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0, 85, 85,  0,  0,  0,  0,  0, 21,127,253, 84,  0,  0,
                 0,  5, 80, 60, 15,  5, 80,  0,  0, 23,255,255,255,255,212,
                 0,  0, 21,255,255,255,255, 84,  0,  0,  1, 85, 85, 85, 85,
                64,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0),
               (31,  0, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0, 85, 85,  0,  0,  0,  0,  0, 21,127,253, 84,  0,  0,
                 0,  5, 92, 15,  3,197, 80,  0,  0, 23,255,255,255,255,212,
                 0,  0, 21,255,255,255,255, 84,  0,  0,  1, 85, 85, 85, 85,
                64,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0),
               (31,  0, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0, 85, 85,  0,  0,  0,  0,  0, 21,127,253, 84,  0,  0,
                 0,  5, 95,  3,192,245, 80,  0,  0, 23,255,255,255,255,212,
                 0,  0, 21,255,255,255,255, 84,  0,  0,  1, 85, 85, 85, 85,
                64,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0),
               (31,  0, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0, 85, 85,  0,  0,  0,  0,  0, 21,127,253, 84,  0,  0,
                 0,  5, 83,192,240, 53, 80,  0,  0, 23,255,255,255,255,212,
                 0,  0, 21,255,255,255,255, 84,  0,  0,  1, 85, 85, 85, 85,
                64,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0),
               (31,  0, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0, 85, 85,  0,  0,  0,  0,  0, 21,127,253, 84,  0,  0,
                 0,  5, 80,240, 60,  5, 80,  0,  0, 23,255,255,255,255,212,
                 0,  0, 21,255,255,255,255, 84,  0,  0,  1, 85, 85, 85, 85,
                64,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0));
  UfoLoesch           :           Array [1..135] OF Byte =
               (31,  0, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0);
  Leben               :           Array [1..49] OF Byte =
               (17,  0,  6,  0, 80,  5, 80,  0,  0,124,  3,245, 64,  0, 90,
               128,224, 10,  0,  2,170,170,170,160,  0, 43,253, 64,  0,  0,
                 3,212,  0,  0,  0,  5, 64,  0,  0, 14,190,174,174,171,  0,
                 0, 62,175,175);

  LebenLoesch        :            Array [1..49] OF Byte =
               (17,  0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0);

  Barrel             :            Array [1..119] OF Byte =
               (24,  0, 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,
               192,  0,  0,  0,  0, 15,255,240,  0,  0,  0, 63,250,186,172,
                 0,  0,  3,250,190,190,175,  0,  0, 14,190,174,174,171,  0,
                 0, 62,175,175,171,171,  0,  0, 58,171,171,235,171,  0,  0,
                58,171,235,235,175,  0,  0, 62,171,235,174,188,  0,  0, 15,
               171,175,255,192,  0,  0,  3,255,255,  0,  0,  0,  0,  0, 63,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0);

  Ship1               :            Array [1..247] OF Byte =
               (39,  0, 23,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 42,170,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,255,192,  0,  0,  0,  0,  0,
                 0,  0,  0, 63,252,  0,  0,  0,  0,  0,  0,  0,  0, 15,255,
               197, 85,  0,  0,  0,  0,  0,  0,  3,245, 84,  1, 80,  0,  0,
                 0,  0,  0,  5, 85,  0,  1, 84,  0,  0,  0,  0, 21, 85, 64,
                 0,  5, 84,  0,  0,  0, 21, 85, 85, 85, 85, 85, 80,  0,  0,
                42, 85, 85, 85,255,255,214,128,  0,  0,  2,170,170,171,255,
               254,170,  0,  0,  0,  0,  2,170,175,255,234,128,  0,  0,  0,
                 0,  0,  0, 63,252,  0,  0,  0,  0,  0,  0,  0,  0,255,192,
                 0,  0,  0,  0,  0,  0,  0,170,170,128,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0);

  Wuerfel              :            Array [0..3,1..191] OF Byte =
              ((28,  0, 22,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,
               170,168,  0,  0,  0,  0, 10,171,250,184,  0,  0,  0,  0, 43,
               255,250,248,  0,  0,  0,  0,175,255,235,248,  0,  0,  0,  2,
               191,255,175,254,  0,  0,  0, 10,250,170,191,254,  0,  0,  0,
                42,175,254,255,254,  0,  0,  0, 47,255,254,255,254,  0,  0,
                 0, 47,255,254,255,254,  0,  0,  0, 47,255,255,191,254,128,
                 0,  0, 11,255,255,191,255,128,  0,  0, 11,255,255,191,255,
               128,  0,  0, 11,255,255,191,255,128,  0,  0, 11,255,255,191,
               254,128,  0,  0, 11,255,255,191,250,  0,  0,  0, 11,255,255,
               191,232,  0,  0,  0, 11,255,255,191,160,  0,  0,  0, 11,255,
               255,238,128,  0,  0,  0,  2,255,255,234,  0,  0,  0,  0,  2,
               255,170,168,  0,  0,  0,  0,  2,170,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0),
               (28,  0, 22,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 10,
               168,  0,  0,  0,  0,  0, 42,171,250,  0,  0,  0,  0, 42,191,
               255,254,128,  0,  0,  0, 47,255,255,255,160,  0,  0,  0, 43,
               255,255,255,232,  0,  0,  0, 42,255,255,255,250,  0,  0,  0,
                42,191,255,255,254,128,  0,  0, 47,175,255,255,250,128,  0,
                 0, 47,235,250,170,175,128,  0,  0, 47,250,175,255,255,128,
                 0,  0, 11,251,255,255,255,128,  0,  0, 11,251,255,255,255,
               128,  0,  0, 11,251,255,255,255,128,  0,  0, 11,251,255,255,
               255,224,  0,  0, 10,251,255,255,255,224,  0,  0,  2,190,255,
               255,255,224,  0,  0,  0,174,255,255,255,224,  0,  0,  0, 42,
               255,255,254,160,  0,  0,  0, 10,254,170,168,  0,  0,  0,  0,
                 2,168,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0),
               (28,  0, 22,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
               170,  0,  0,  0,  0,  0,170,170,254,128,  0,  0,  0, 42,255,
               255,254,160,  0,  0,  0, 47,255,255,254,232,  0,  0,  0, 47,
               255,255,254,250,  0,  0,  0, 47,255,255,255,190,128,  0,  0,
                47,255,255,255,191,128,  0,  0, 11,255,255,255,191,128,  0,
                 0, 11,255,255,255,191,128,  0,  0, 11,255,255,255,191,128,
                 0,  0, 11,255,255,234,191,224,  0,  0, 11,234,170,191,175,
               224,  0,  0, 10,191,255,255,235,224,  0,  0, 10,255,255,255,
               250,160,  0,  0,  2,191,255,255,254,160,  0,  0,  0,175,255,
               255,255,160,  0,  0,  0, 43,255,255,255,224,  0,  0,  0, 10,
               255,255,250,160,  0,  0,  0,  2,191,170,160,  0,  0,  0,  0,
                 0,170,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0),
               (28,  0, 22,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                10,168,  0,  0,  0,  0,  2,170,175,248,  0,  0,  0,  0, 10,
               191,255,248,  0,  0,  0,  0, 43,191,255,254,  0,  0,  0,  0,
               175,239,255,254,  0,  0,  0,  2,191,239,255,254,  0,  0,  0,
                10,255,239,255,254,  0,  0,  0, 43,255,239,255,254,  0,  0,
                 0, 47,255,239,255,254,  0,  0,  0, 47,255,239,255,254,  0,
                 0,  0, 47,255,239,255,254,  0,  0,  0, 43,255,239,255,255,
               128,  0,  0, 11,255,251,255,255,128,  0,  0, 11,255,251,255,
               255,128,  0,  0, 11,255,251,255,170,128,  0,  0, 11,255,234,
               170,250,  0,  0,  0, 11,255,175,255,232,  0,  0,  0,  2,254,
               191,255,160,  0,  0,  0,  2,250,255,254,128,  0,  0,  0,  2,
               234,254,170,  0,  0,  0,  0,  2,170,168,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0));
  Brocken               :             Array [0..3,1..91] OF Byte =
              ((20,  0, 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  2,130,  0,  0,  0,  0,  9,105,
                90,128,  0,  0, 38,181,229,112,  0,  0, 38,157,102,152,  0,
                 0, 10,153,221,218,  0,  0,  9,102,169,245,  0,  0, 38,167,
               101,168,  0,  0,  1,102,150,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                11),
               (20,  0, 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,252,  0,  0,  0,  0, 15,106,
               193,188,  0,  0, 39,173,217,104,  0,  0, 37,155,102, 89,  0,
                 0, 11,151,158,107,  0,  0,  1,103,125,252,  0,  0,  0,167,
                85, 96,  0,  0,  0,  8,152, 64,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                11),
               (20,  0, 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,130, 12,  0,  0,  0,  1,189,
               107,140,  0,  0,  2,182, 86,248,  0,  0,  6,217,170,212,  0,
                 0, 10,217,215,184,  0,  0,  1,102,233,160,  0,  0,  2,166,
                94,192,  0,  0,  0, 62,164,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                11),
               (20,  0, 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  3,  0,  0,  0,  0,169,
               217,128,  0,  0,  2,101,229,120,  0,  0,  7,155,173,223,  0,
                 0,  3,213,214, 86,  0,  0,  9,101, 85,186,  0,  0,  2, 87,
               231,176,  0,  0,  0,  6,154, 80,  0,  0,  0,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                11));



PROCEDURE ExplainGame;
PROCEDURE Setup;
PROCEDURE FinishGame;
PROCEDURE SetScreenMode;
PROCEDURE OutTextXY3D (x,y          :      Integer;
                       Text         :      String;
                       StartCol,Anz :      Byte);
PROCEDURE Box (x1,y1,x2,y2      :      Integer);
PROCEDURE HiScoreScreen;
PROCEDURE TitleScreen;
FUNCTION GetKey          :       Word;
PROCEDURE ClearInput;
FUNCTION Key   :     Byte;
PROCEDURE GotHiScore;
FUNCTION MainMenu        :      Byte;


IMPLEMENTATION


PROCEDURE Init;
BEGIN
  GraphDriver := 1;
  GraphMode := 1;
  InitGraph (GraphDriver,GraphMode,'');
  SetGraphMode (GraphMode);
  SetFillStyle (SolidFill,0);
  Randomize;
  Finished := False;
  MenuScreenLoaded := False;
  {$I-}
  Assign (ScreenDatei,'SCREEN1.SPC');
  Reset (ScreenDatei);
  Read (ScreenDatei,MenuScreen);
  IF IOResult = 0 THEN
  BEGIN
    Close (ScreenDatei);
    IF IOResult = 0 THEN MenuScreenLoaded := True;
  END;
END;


PROCEDURE SetScreenMode;
BEGIN
  SetGraphMode (GraphMode);
  SetFillStyle (SolidFill,0);
END;


PROCEDURE OutTextXY3D (x,y          :      Integer;
                       Text         :      String;
                       StartCol,Anz :      Byte);
VAR
  i,col               :           Byte;


BEGIN
  col := StartCol;
  FOR i := 0 TO Pred (anz) DO
  BEGIN
    SetColor (col);
    col := Succ (Succ (col) MOD 3);
    OutTextXY (x + i,y + i,Text);
  END;
END;


PROCEDURE Box (x1,y1,x2,y2      :      Integer);
BEGIN
  MoveTo (x1,y1);
  LineTo (x2,y1);
  LineTo (x2,y2);
  LineTo (x1,y2);
  LineTo (x1,y1);
END;


PROCEDURE LoadHighScores;

VAR
  r              :        Real;
  check          :        Integer;
  hstr           :        String [5];

BEGIN
  Assign (Datei,'SPACE.HSC');
  {$I-}
  Reset (Datei);
  FOR i := 1 TO 8 DO Readln (Datei,HiScore [i],Name [i]);
  Readln (Datei,hstr);
  Val (hstr,SpeedDelay,Check);
  IF Check <> 0 THEN SpeedDelay := 0;
  Readln (Datei,hstr);
  IF hstr = '0' THEN SoundOn := False ELSE SoundOn := True;
  i := IOResult;
  IF i = 0 THEN Close (Datei);
  IF i <> 0 THEN
  BEGIN
    FOR i := 1 TO 8 DO
    BEGIN
      r := (9 - i) * 3000.0;
      Str (r:8:0,HiScore [i]);
      Name [i] := '--- ABCS ---';
    END;
  END;
END;


PROCEDURE SaveHighScores;

VAR
  hstr       :      String [5];

BEGIN
  Assign (Datei,'SPACE.HSC');
  {$I-}
  Rewrite (Datei);
  FOR i := 1 TO 8 DO Writeln (Datei,HiScore [i],Name [i]);
  Str (SpeedDelay,hstr);
  Writeln (Datei,hstr);
  IF SoundOn THEN Writeln (Datei,'1') ELSE Writeln (Datei,'0');
  i := IOResult;
  Close (Datei);
  {$I+}
END;


PROCEDURE MovePageStars;

VAR
  i          :        Word;

BEGIN
  FOR i := 1 TO MaxPageStar DO
  BEGIN
    PutPixel (OldSStarX [i] SHR 2,OldSStarY [i] SHR 2,0);
    IF (GetPixel (SStarX [i] SHR 2,SStarY [i] SHR 2) = 0) THEN
    BEGIN
      PutPixel (SStarX [i] SHR 2,SStarY [i] SHR 2,3);
      OldSStarX [i] := SStarX [i];
      OldSStarY [i] := SStarY [i];
    END ELSE OldSStarX [i] := 1290;
    SStarX [i] := SStarX [i] + SStarMoveX [i];
    SStarY [i] := SStarY [i] + SStarMoveY [i];
    IF (SStarX [i] < 4) OR (SStarX [i] > 1280) OR
    (SStarY [i] < 4) OR (SStarY [i] > 800) THEN
    BEGIN
      PutPixel (OldSStarX [i] SHR 2,OldSStarY [i] SHR 2,0);
      SStarX [i] := (sx + 12) SHL 2;
      SStarY [i] := (sy + 12) SHL 2;
      SStarMoveX [i] := (Random (45) - 22) * 2;
      SStarMoveY [i] := (Random (45) - 22) * 2;
      IF SStarMoveX [i] = 0 THEN SStarMoveX [i] := 9;
    END;
  END;
END;


PROCEDURE InitPageStars;

VAR
  i           :      Byte;

BEGIN
  FOR i := 1 TO MaxPageStar DO
  BEGIN
    SStarX [i] := sx SHL 2;
    SStarY [i] := sy SHL 2;
    SStarMoveX [i] := Random (45) - 22;
    SStarMoveY [i] := Random (45) - 22;
    OldSStarX [i] := 1290;
  END;
END;


PROCEDURE HiScoreScreen;

VAR
  hstr         :      String [1];

BEGIN
  Bar (0,0,319,199);
  SetColor (3);
  Box (0,0,319,199);
  SetColor (2);
  Box (1,1,318,198);
  SetTextStyle (TriplexFont,HorizDir,0);
  SetTextJustify (LeftText,LeftText);
  SetUserCharSize (12,16,9,16);
  SetColor (1);
  OutTextXY (11,16,'Spaceflight');
  SetColor (2);
  OutTextXY (12,17,'Spaceflight  Hiscores :');
  SetColor (3);
  OutTextXY (13,18,'Spaceflight');
  SetUserCharSize (9,16,8,16);
  FOR i := 1 TO 8 DO
  BEGIN
    SetColor (Succ (i MOD 3));
    Str (i,hstr);
    OutTextXY (15,25 + i * 18,hstr + '.');
    OutTextXY (40,25 + i * 18,Name [i]);
    OutTextXY (210,25 + i * 18,Hiscore [i]);
  END;
  SetColor (2);
  SetUserCharSize (10,16,8,16);
  OutTextXY (10,186,'press any key to continue...');
  sx := 160;
  sy := 100;
  InitPageStars;
  Repeat
    MovePageStars;
  UNTIL Keypressed;
  a := ReadKey;
END;


PROCEDURE TitleScreen;

CONST
  TextString             =        'ABCS';
  MaxSchriftGroesse      =         16;
  oben                   =          1;
  links                  =          1;
  unten                  =        131;
  rechts                 =        238;


VAR
  SizeAdd,
  xpos,ypos,
  xadd,yadd,
  zxpos,zypos,
  y,k          :         Integer;
  BildSize     :         Word;
  BildPuffer   :         Array [1..MaxSchriftGroesse,0..3200] OF Byte;


BEGIN
  SetLineStyle (0,0,1);
  SetFillStyle (SolidFill,0);
  SetColor (3);
  FOR k := 1 TO MaxSchriftGroesse DO
  BEGIN
    Bar (120,52,200,90);
    SetTextStyle (TriplexFont,HorizDir,0);
    SetUserCharSize (k,16,k,16);
    SetTextJustify (CenterText,CenterText);
    SetColor (Succ (k MOD 3));
    OutTextXY3D (159,67,TextString,Succ (k MOD 3),3);
    GetImage (120,52,200,90,BildPuffer [k]);
  END;
  Bar (120,52,200,90);
  SetColor (3);
  Box (0,0,319,199);
  SetColor (2);
  Box (1,1,318,198);
  SetTextJustify (LeftText,LeftText);
  SetUserCharSize (9,16,9,16);
  SetColor (2);
  OutTextXY (10,186,'ABCS presents :');
  SetUserCharSize (14,16,12,16);
  OutTextXY3D (153,185,'Spaceflight',1,6);
  BildSize := 1;
  SizeAdd := 1;
  xpos := 4 + Random (100);
  ypos := 4 + Random (100);
  xadd := 1;
  yadd := 1;
  sx := 100;
  sy := 70;
  InitPageStars;
  Repeat
    PutImage (xpos,ypos,BildPuffer [BildSize],CopyPut);
    BildSize := BildSize + SizeAdd;
    IF BildSize <= 1 THEN SizeAdd := 1;
    IF BildSize >= MaxSchriftGroesse THEN SizeAdd := -1;
    zxpos := xpos;
    xpos := xpos + xadd;
    IF (xpos <= links) OR (xpos >= rechts) THEN
    BEGIN
      xadd := - xadd;
      xpos := zxpos;
    END;
    zypos := ypos;
    ypos := ypos + yadd;
    IF (ypos <= oben) OR (ypos >= unten) THEN
    BEGIN
      yadd := - yadd;
      ypos := zypos;
    END;
    MovePageStars;
  Until KeyPressed;
  a := ReadKey;
END;


FUNCTION MainMenu        :      Byte;

VAR
  wahl           :          Byte;
  Check          :          Integer;

BEGIN
  SetScreenMode;
  IF MenuScreenLoaded THEN
  BEGIN
    PutImage (0,0,MenuScreen,CopyPut);
  END
  ELSE
  BEGIN
    SetTextStyle (TriplexFont,HorizDir,0);
    SetTextJustify (LeftText,LeftText);
    SetColor (3);
    Box (0,0,319,199);
    SetColor (2);
    Box (1,1,318,198);
    SetUserCharSize (14,16,12,16);
    OutTextXY3D (83,25,'Spaceflight',1,6);
    SetUserCharSize (8,16,8,16);
    OutTextXY3D (10,59,'written 1989 by Andreas Bauer (ABCS)',1,3);
    SetUserCharSize (11,16,11,16);
    SetColor (3);
    OutTextXY (8,85,'Main Menu :');
    SetColor (2);
    SetUserCharSize (13,16,9,16);
    OutTextXY (8,105,'-1- Show hiscores');
    OutTextXY (8,120,'-2- Start game');
    OutTextXY (8,135,'-3- Explain game');
    OutTextXY (8,150,'-4- Setup');
    OutTextXY (8,165,'-5- Quit game');
    SetUserCharSize (11,16,11,16);
    OutTextXY3D (15,185,'please make your choice...',1,3);
    GetImage (0,0,319,199,MenuScreen);
    MenuScreenLoaded := True;
    Assign (ScreenDatei,'SCREEN1.SPC');
    Rewrite (ScreenDatei);
    Write (ScreenDatei,MenuScreen);
    IF IOResult <> 0 THEN
    BEGIN
      Close (ScreenDatei);
    END;
  END;
  sx := 190;
  sy := 70;
  InitPageStars;
  wahl := 0;
  Repeat
    MovePageStars;
    IF Keypressed THEN
    BEGIN
      a := ReadKey;
      Val (a,wahl,Check);
    END;
  UNTIL (wahl > 0) AND (wahl < 6) AND (Check = 0);
  MainMenu := wahl;
END;


PROCEDURE ClearInput;
BEGIN
  While Keypressed DO
  a := ReadKey;
END;


FUNCTION GetKey          :       Word;

VAR
  zeichen         :       Word;

BEGIN
  zeichen := Ord (ReadKey);
  IF zeichen = 0 THEN zeichen := 256 + Ord (ReadKey);
  GetKey := zeichen;
END;


FUNCTION Key   :     Byte;
BEGIN
  Reg.ah := 0;
  INTR ($16,Reg);
  Key := Reg.ah;
END;


FUNCTION GraphRead (x,y      :     Integer)        :       String;

VAR
  xpos,ypos         :       Integer;
  zeichen           :       Byte;
  InputStr          :       String [255];

BEGIN
  InputStr := '';
  xpos := x;
  ypos := y;
  Repeat
    Line (xpos,ypos - 14,xpos,ypos + 2);
    zeichen := GetKey;
    CASE zeichen OF
  8 : BEGIN
        IF Length (InputStr ) > 0 THEN
        BEGIN
          Delete (InputStr,Length (InputStr),1);
          Bar (xpos - 2,ypos + 12,xpos + 16,ypos - 16);
          xpos := xpos - 16;
          Bar (xpos - 2,ypos + 12,xpos + 16,ypos - 16);
        END;
        zeichen := 0;
      END;
    END;
    IF zeichen > 31 THEN
    BEGIN
      Bar (xpos - 2,ypos + 12,xpos + 16,ypos - 16);
      OutTextXY3D (xpos - 1,ypos - 1,Chr (zeichen),1,3);
      xpos := xpos + 16;
      InputStr := InputStr + Chr (zeichen);
    END;
  UNTIL zeichen = 13;
  Bar (xpos,ypos + 12,xpos + 16,ypos - 16);
  GraphRead := InputStr;
END;


PROCEDURE GotHiScore;

VAR
  r           :       LongInt;
  hstr        :       String [8];

BEGIN
  SetScreenMode;
  SetTextStyle (TriplexFont,HorizDir,0);
  SetTextJustify (LeftText,LeftText);
  SetColor (3);
  ClearInput;
  Box (0,0,319,199);
  SetColor (2);
  Box (1,1,318,198);
  Val (HiScore [8],r,i);
  Str (Score,hstr);
  SetUserCharSize (11,16,10,16);
  SetColor (1);
  OutTextXY (9,19,'All your ships are destroyed.');
  SetUserCharSize (8,16,10,16);
  SetColor (2);
  OutTextXY (9,39,'You have achieved');
  OutTextXY3D (157,38,hstr + ' points.',1,3);
  IF Score >= r THEN
  BEGIN
    SetColor (1);
    SetUserCharSize (9,16,10,16);
    OutTextXY (19,70,'Therefore you get a place in the');
    OutTextXY (19,88,'Hiscore table. Please enter your');
    OutTextXY (19,106,'name (up to 12 characters) .');
    SetColor (2);
    OutTextXY (9,160,'Your name : ');
    SetColor (3);
    Nameein := GraphRead (125,160);
    FOR i := 7 DownTo 1 DO
    BEGIN
      Val (HiScore [i],r,j);
      IF Score >= r THEN
      BEGIN
        HiScore [Succ (i)] := HiScore [i];
        Name [Succ (i)] := Name [i];
      END
      ELSE
      BEGIN
        Str (Score:8,HiScore [Succ (i)]);
        Name [Succ (i)] := Nameein;
        Score := 0;
        i := 1;
      END;
    END;
    IF Score >= r THEN
    BEGIN
      Str (Score:8,HiScore [i]);
      Name [i] := Nameein;
    END;
  END
  ELSE
  BEGIN
    SetUserCharSize (10,16,12,16);
    OutTextXY (14,183,'press any key to continue...');
    a := ReadKey;
  END;
END;


PROCEDURE ExplainGame;
BEGIN
  Bar (2,2,317,197);
  SetTextStyle (DefaultFont,HorizDir,0);
  SetTextJustify (CenterText,CenterText);
  OutTextXY3D (158,15,'How to play SPACEFLIGHT',1,3);
  OutTextXY3D (159,50,'You have to move the spaceliner AEROFOX',1,2);
  OutTextXY3D (159,65,'through the endless width of the space',1,2);
  OutTextXY3D (159,80,'to bring important medicaments to the',1,2);
  OutTextXY3D (159,95,'planet ENOLA. But there are lots of',1,2);
  OutTextXY3D (159,110,'meteorits flying around which destroy',1,2);
  OutTextXY3D (159,125,'your ship if you get too close.',1,2);
  OutTextXY3D (159,140,'Another danger to your spaceship are',1,2);
  OutTextXY3D (159,155,'the pirates who attack every trader,and',1,2);
  OutTextXY3D (159,170,'UFOs which approach foreign spaceships',1,2);
  OutTextXY3D (159,185,'in order to exstinguish them.',1,2);
  sx := 160;
  sy := 100;
  InitPageStars;
  Repeat
    MovePageStars;
  UNTIL Keypressed;
  a := ReadKey;
  Bar (2,2,317,197);
  OutTextXY3D (158,15,'Keys you need for ship control',1,3);
  OutTextXY3D (159,50,'You can use all cursorkeys including',1,2);
  OutTextXY3D (159,65,'Home,PgUp,End and PgDn to move the',1,2);
  OutTextXY3D (159,80,'spaceship. The space-key is used',1,2);
  OutTextXY3D (159,95,'to fire a shoot.',1,2);
  OutTextXY3D (159,110,'After pressing ESC the game will',1,2);
  OutTextXY3D (159,125,'be paused and you can continue',1,2);
  OutTextXY3D (159,140,'playing with any key or quit',1,2);
  OutTextXY3D (159,155,'by pressing ESC once again.',1,2);
  sx := 160;
  sy := 100;
  InitPageStars;
  Repeat
    MovePageStars;
  UNTIL Keypressed;
  a := ReadKey;
  Bar (2,2,317,197);
  OutTextXY3D (158,15,'objects you will meet in space',1,3);
  PutImage (2,45,Jet,CopyPut);
  SetTextJustify (LeftText,CenterText);
  OutTextXY3D (60,50,'Carriership "TURBO BACS".',1,2);
  OutTextXY3D (60,65,'This is the ship you have to',1,2);
  OutTextXY3D (60,80,'control.',1,2);
  PutImage (15,120,Ufo [0],CopyPut);
  OutTextXY3D (60,105,'Unknown Flying Object (UFO).',2,2);
  OutTextXY3D (60,120,'These small ships try to',2,2);
  OutTextXY3D (60,135,'destroy other ships by',2,2);
  OutTextXY3D (60,150,'crashing into them. More',2,2);
  OutTextXY3D (60,165,'information is not available.',2,2);
  sx := 160;
  sy := 100;
  InitPageStars;
  Repeat
    MovePageStars;
  UNTIL Keypressed;
  a := ReadKey;
  Bar (2,2,317,197);
  SetTextJustify (CenterText,CenterText);
  OutTextXY3D (158,15,'objects you will meet in space',1,3);
  SetTextJustify (LeftText,CenterText);
  PutImage (10,60,Ship1,CopyPut);
  OutTextXY3D (60,50,'Fighter "ACOMBA". Has two guns',3,2);
  OutTextXY3D (60,65,'which can fire oxygen',3,2);
  OutTextXY3D (60,80,'rockets. Ship is considered',3,2);
  OutTextXY3D (60,95,'to be very hazardous.',3,2);
  PutImage (12,115,Meteorit,CopyPut);
  OutTextXY3D (60,115,'Meteorit. Large rock which',1,2);
  OutTextXY3D (60,130,'can be found nearly everywhere.',1,2);
  OutTextXY3D (60,145,'in space. Can destroy spaceships',1,2);
  OutTextXY3D (60,160,'if they approach.',1,2);
  sx := 160;
  sy := 100;
  InitPageStars;
  Repeat
    MovePageStars;
  UNTIL Keypressed;
  a := ReadKey;
  Bar (2,2,317,197);
  SetTextJustify (CenterText,CenterText);
  OutTextXY3D (158,15,'objects you will meet in space',1,3);
  SetTextJustify (LeftText,CenterText);
  PutImage (10,60,Barrel,CopyPut);
  OutTextXY3D (60,50,'Barrel filled with fuel.',2,2);
  OutTextXY3D (60,65,'Shooting at barrel will',2,2);
  OutTextXY3D (60,80,'refill your ships fuel reserve.',2,2);
  PutImage (10,115,Wuerfel [0],CopyPut);
  OutTextXY3D (60,115,'CSS (Cube Space Ship)',3,2);
  OutTextXY3D (60,130,'The crews of these ships',3,2);
  OutTextXY3D (60,145,'fix broken planes. Shoot',3,2);
  OutTextXY3D (60,160,'at them to get a new ship.',3,2);
  sx := 160;
  sy := 100;
  InitPageStars;
  Repeat
    MovePageStars;
  UNTIL Keypressed;
  a := ReadKey;
  Bar (2,2,317,197);
  SetTextJustify (CenterText,CenterText);
  OutTextXY3D (158,15,'tips and hints for space pilots',1,3);
  SetTextJustify (CenterText,CenterText);
  OutTextXY3D (158,50,'Always watch the fuel display',2,2);
  OutTextXY3D (158,65,'(the long bar in the upper screen).',2,2);
  OutTextXY3D (158,80,'Flying and shooting consumes fuel.',2,2);
  OutTextXY3D (158,95,'If there is no fuel left at all your',2,2);
  OutTextXY3D (158,110,'ship will be destroyed.',2,2);
  OutTextXY3D (158,125,'Sometimes clouds of small rocks',2,2);
  OutTextXY3D (158,140,'appear on screen. Try keeping away',2,2);
  OutTextXY3D (158,155,'from them because shooting at those',2,2);
  OutTextXY3D (158,170,'rocks has no effect.',2,2);
  sx := 160;
  sy := 100;
  InitPageStars;
  Repeat
    MovePageStars;
  UNTIL Keypressed;
  a := ReadKey;
  Bar (2,2,317,197);
  SetTextJustify (CenterText,CenterText);
  OutTextXY3D (158,15,'tips and hints for space pilots',1,3);
  SetTextJustify (CenterText,CenterText);
  OutTextXY3D (158,50,'If you want to stop your ship',3,2);
  OutTextXY3D (158,65,'moving you can press all keys',3,2);
  OutTextXY3D (158,80,'except those you need for ship',3,2);
  OutTextXY3D (158,95,'control.',3,2);
  OutTextXY3D (158,110,'You can toggle the sound on or',3,2);
  OutTextXY3D (158,125,'off using the setup option.',3,2);
  OutTextXY3D (158,140,'Slowing down the game is possible',3,2);
  OutTextXY3D (158,155,'by entering a number at the speed',3,2);
  OutTextXY3D (158,170,'option of the setup menu.',3,2);
  OutTextXY3D (158,185,'A big number means a slow game.',3,2);
  sx := 160;
  sy := 100;
  InitPageStars;
  Repeat
    MovePageStars;
  UNTIL Keypressed;
  a := ReadKey;
END;


PROCEDURE Setup;

VAR
  hstr         :      String [5];
  wahl         :      Byte;
  Check        :      Integer;

BEGIN
  Bar (2,2,317,197);
  SetTextStyle (TriplexFont,HorizDir,0);
  SetTextJustify (CenterText,CenterText);
  SetUserCharSize (16,16,16,16);
  OutTextXY3D (158,20,'Setup',1,3);
  SetTextJustify (LeftText,LeftText);
  SetUserCharSize (12,16,12,16);
  SetColor (2);
  OutTextXY (10,80,'-1- Sound : ');
  IF SoundOn THEN OutTextXY3D (170,80,'ON',1,3)
  ELSE OutTextXY3D (170,80,'OFF',1,3);
  Str (SpeedDelay,hstr);
  SetColor (2);
  OutTextXY (10,110,'-2- Speed :');
  OutTextXY3D (170,110,hstr,2,3);
  SetColor (2);
  OutTextXY (10,140,'-3- Main Menu');
  SetUserCharSize (11,16,11,16);
  OutTextXY3D (15,185,'please make your choice...',1,3);
  sx := 159;
  sy := 100;
  InitPageStars;
  wahl := 0;
  Repeat
    wahl := 0;
    Repeat
      MovePageStars;
      IF Keypressed THEN
      BEGIN
        a := ReadKey;
        Val (a,wahl,Check);
      END;
    UNTIL (wahl > 0) AND (wahl < 4) AND (Check = 0);
    CASE wahl OF
  1 : BEGIN
        IF SoundOn THEN
        BEGIN
          SoundOn := False;
          Bar (165,85,230,60);
          OutTextXY3D (170,80,'OFF',1,3);
        END
        ELSE
        BEGIN
          SoundOn := True;
          Bar (165,85,230,60);
          OutTextXY3D (170,80,'ON',1,3);
        END;
      END;
  2 : BEGIN
        Repeat
          Bar (165,115,300,90);
          hstr := GraphRead (170,110);
          Val (hstr,SpeedDelay,Check);
        UNTIL Check = 0;
      END;
    END;
  UNTIL wahl = 3;
END;


PROCEDURE FinishGame;

LABEL
  Key;

BEGIN
  Bar (0,0,319,199);
  SetTextStyle (TriplexFont,HorizDir,0);
  SetTextJustify (LeftText,LeftText);
  SetUserCharSize (13,16,16,16);
  SetColor (2);
  OutTextXY (4,30,'Thanks for playing');
  SetUserCharSize (16,16,16,16);
  FOR i := 1 TO 5 DO
  BEGIN
    SetColor (Succ (i MOD 3));
    OutTextXY (45 + i,75 + i ,'Spaceflight');
    IF Keypressed THEN Goto Key;
  END;
  FOR i := 1 TO 5 DO
  BEGIN
    SetColor (Succ (i MOD 3));
    OutTextXY (65 + i,125 + i ,'Spaceflight');
    IF Keypressed THEN Goto Key;
  END;
  NoSound;
  SetUserCharSize (10,16,16,16);
  OutTextXY (4,183,'press any key to continue...');
  Key :
  sx := 160;
  sy := 100;
  InitPageStars;
  Repeat
    MovePageStars;
  UNTIL Keypressed;
  a := ReadKey;
  Finished := True;
  NoSound;
  SaveHighScores;
END;


BEGIN
  Init;
  LoadHighScores;
  SetScreenMode;
  TitleScreen;
END.