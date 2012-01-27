<map version="0.9.0">
<!-- To view this file, download free mind mapping software FreeMind from http://freemind.sourceforge.net -->
<node COLOR="#111111" CREATED="1317404294590" ID="ID_1025087291" MODIFIED="1319716495692" STYLE="bubble" TEXT="StarCraft AI" VGAP="65">
<edge COLOR="#111111" WIDTH="thin"/>
<font BOLD="true" NAME="SansSerif" SIZE="18"/>
<node CREATED="1317404580265" ID="ID_546022262" MODIFIED="1319716712628" POSITION="right" TEXT="What should it do?" VGAP="54">
<font BOLD="true" NAME="SansSerif" SIZE="16"/>
<node CREATED="1317411757954" ID="ID_1162545605" MODIFIED="1319622515649" TEXT="Communication" VGAP="15">
<font NAME="SansSerif" SIZE="14"/>
<node CREATED="1317411766810" ID="ID_1689435221" MODIFIED="1319707789355" TEXT="Communicate it&apos;s intentions to the player">
<font NAME="SansSerif" SIZE="12"/>
<node CREATED="1317411808954" ID="ID_1100775062" MODIFIED="1319622475793" TEXT="What it is building and why"/>
<node CREATED="1317411823938" ID="ID_1236110767" MODIFIED="1319622475793" TEXT="Why it is counter-attacking instead of helping the player, etc."/>
</node>
<node CREATED="1319629502192" ID="ID_45420681" MODIFIED="1319707793794" TEXT="Uses minimap to specify where to do the action (if applicable)"/>
<node CREATED="1319629516835" ID="ID_1487759657" MODIFIED="1319629522594" TEXT="Commands (for the player)">
<node CREATED="1317412163778" ID="ID_554188035" MODIFIED="1319707786893" TEXT="Actions">
<font NAME="SansSerif" SIZE="12"/>
<node CREATED="1317412356538" ID="ID_1690726283" MODIFIED="1319622475805" TEXT="Ask for money"/>
<node CREATED="1317411371162" ID="ID_313926801" MODIFIED="1319622475805" TEXT="Attack"/>
<node CREATED="1317411427922" ID="ID_1901693604" MODIFIED="1319622475805" TEXT="Counter-attack"/>
<node CREATED="1317411421066" ID="ID_1955379491" MODIFIED="1319622475804" TEXT="Drop"/>
<node CREATED="1317412576746" ID="ID_1466776051" MODIFIED="1319622475804" TEXT="Expand"/>
<node CREATED="1317411424010" ID="ID_489063265" MODIFIED="1319622475804" TEXT="Harass"/>
<node CREATED="1317411416562" ID="ID_1028260663" MODIFIED="1319622475804" TEXT="Move To"/>
<node CREATED="1317412722274" ID="ID_1936439608" MODIFIED="1319622475804" TEXT="Scout"/>
</node>
<node CREATED="1317412166466" ID="ID_14132702" MODIFIED="1319707785086" TEXT="Strategy">
<font NAME="SansSerif" SIZE="12"/>
<node CREATED="1317412395810" ID="ID_707297363" MODIFIED="1319622475804" TEXT="Main focus on units" VGAP="19">
<node CREATED="1317412415418" ID="ID_1074589831" MODIFIED="1319622475804" TEXT="Abstract">
<node CREATED="1317412418282" ID="ID_428451467" MODIFIED="1319622475804" TEXT="Air"/>
<node CREATED="1317412468986" ID="ID_985883202" MODIFIED="1319622475804" TEXT="Ground"/>
<node CREATED="1317412470770" ID="ID_1664458460" MODIFIED="1319622475803" TEXT="Support"/>
</node>
<node CREATED="1317412427666" ID="ID_510135138" MODIFIED="1319622556848" TEXT="Specific">
<node CREATED="1317412474946" ID="ID_669576048" MODIFIED="1319622475803" TEXT="Tanks"/>
<node CREATED="1317412482234" ID="ID_564833500" MODIFIED="1319622475803" TEXT="Medics"/>
<node CREATED="1317412489322" ID="ID_209086169" MODIFIED="1319622475803" TEXT="Marines"/>
<node CREATED="1317412491546" ID="ID_1681240823" MODIFIED="1319622475803" TEXT="etc."/>
</node>
</node>
</node>
<node CREATED="1319706707644" ID="ID_1498765382" MODIFIED="1319706726122" TEXT="Should the action time-out?">
<icon BUILTIN="help"/>
</node>
<node CREATED="1319706868479" ID="ID_819733828" MODIFIED="1319706901741">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      How much time do we wait until the next one
    </p>
    <p>
      after a command was finished?
    </p>
  </body>
</html></richcontent>
<icon BUILTIN="help"/>
<node CREATED="1319706905706" ID="ID_619575623" MODIFIED="1319706957850" TEXT="This will enable player analysis again">
<arrowlink DESTINATION="ID_264083774" ENDARROW="Default" ENDINCLINATION="283;-180;" ID="Arrow_ID_479356601" STARTARROW="None" STARTINCLINATION="22;363;"/>
</node>
</node>
</node>
</node>
<node COLOR="#000000" CREATED="1317411456258" ID="ID_397378692" MODIFIED="1319716754925" TEXT="Adaptiveness">
<font NAME="SansSerif" SIZE="14"/>
<node CREATED="1317411464626" ID="ID_935597281" MODIFIED="1319630166950" TEXT="Adapts to the same level as the player">
<font NAME="SansSerif" SIZE="12"/>
<node CREATED="1319621150388" ID="ID_762008693" MODIFIED="1319622475802" TEXT="How should this be done?">
<icon BUILTIN="help"/>
</node>
<node CREATED="1319621167580" ID="ID_1710106960" MODIFIED="1319630188282" TEXT="Can be done that so that it creates units and&#xa;develops technology in roughly the same pase&#xa;as the player">
<icon BUILTIN="idea"/>
</node>
</node>
</node>
<node CREATED="1319630055358" ID="ID_264083774" MODIFIED="1319706957850" TEXT="Player and enemy analysis">
<arrowlink DESTINATION="ID_1948409082" ENDARROW="Default" ENDINCLINATION="14;-27;" ID="Arrow_ID_1814469343" STARTARROW="None" STARTINCLINATION="-11;22;"/>
<font NAME="SansSerif" SIZE="14"/>
<node CREATED="1319630067257" ID="ID_233677184" MODIFIED="1319630082378" TEXT="How to read the player&apos;s behaviors?">
<icon BUILTIN="help"/>
</node>
<node CREATED="1319630083228" ID="ID_1005625350" MODIFIED="1319706857786">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      The AI will not try to guess if the player's intensions changed during the command
    </p>
    <p>
      Instead the command will time-out if nothing is done.
    </p>
  </body>
</html></richcontent>
</node>
</node>
<node CREATED="1317411582490" ID="ID_1948409082" MODIFIED="1319706648743" TEXT="Initiative" VGAP="38">
<arrowlink DESTINATION="ID_1865097336" ENDARROW="Default" ENDINCLINATION="-88;182;" ID="Arrow_ID_988041348" STARTARROW="None" STARTINCLINATION="-289;519;"/>
<font NAME="SansSerif" SIZE="14"/>
<node CREATED="1317411589554" ID="ID_1119134309" MODIFIED="1319622475797" TEXT="How much initiative should it take?">
<font NAME="SansSerif" SIZE="12"/>
<icon BUILTIN="help"/>
</node>
<node CREATED="1317411661010" ID="ID_1816381233" MODIFIED="1319622547648" TEXT="Different levels of initiative" VGAP="23">
<font NAME="SansSerif" SIZE="12"/>
<node CREATED="1317411674674" ID="ID_744770416" MODIFIED="1319706198173" TEXT="High">
<node CREATED="1317411877906" ID="ID_525488371" MODIFIED="1319622475797" TEXT="Comes with suggestions on strategies, such as drops etc and asks if it or the player should do it"/>
<node CREATED="1317411982626" ID="ID_645207610" MODIFIED="1319622475796" TEXT="Always decides to do what it thinks is the best to do in battles&#x2014;can counter-attack"/>
<node CREATED="1317412614402" ID="ID_988472957" MODIFIED="1319622475796" TEXT="Expands when it thinks is the best time"/>
</node>
<node CREATED="1317411690402" ID="ID_1093672673" MODIFIED="1319706213379" TEXT="Medium">
<node CREATED="1317411954474" ID="ID_235954418" MODIFIED="1319622475796" TEXT="Comes with less suggestions."/>
<node CREATED="1317412704242" ID="ID_114992750" MODIFIED="1319622475796" TEXT="Can attack some time"/>
<node CREATED="1317412713298" ID="ID_30029357" MODIFIED="1319622475796" TEXT="Goes around scouting"/>
<node CREATED="1317414921451" ID="ID_1714267300" MODIFIED="1319622475796" TEXT="When the player moves out to attack it also moves out and groups with the player"/>
</node>
<node CREATED="1317411693297" ID="ID_1679520601" MODIFIED="1319706215987" STYLE="bubble" TEXT="Low">
<node CREATED="1317412546762" ID="ID_1577243248" MODIFIED="1319622475796" TEXT="Only comes to aid when the player needs help"/>
<node CREATED="1317412622722" ID="ID_534391353" MODIFIED="1319622475796" TEXT="Only expands when the either the user tells it to, or if it comes above a certain threshold"/>
<node CREATED="1317412688058" ID="ID_1975139832" MODIFIED="1319622475795" TEXT="Never attacks on it own"/>
</node>
</node>
</node>
<node CREATED="1317412075554" ID="ID_770765261" MODIFIED="1319629432850" TEXT="Actions" VGAP="13">
<font NAME="SansSerif" SIZE="14"/>
<node CREATED="1319629440540" ID="ID_1879884413" MODIFIED="1319629446904" TEXT="Action-oriented">
<node CREATED="1317411371162" ID="ID_424053266" MODIFIED="1319622475795" TEXT="Attack">
<font NAME="SansSerif" SIZE="12"/>
</node>
<node CREATED="1317411427922" ID="ID_166812100" MODIFIED="1319622475795" TEXT="Counter-attack">
<font NAME="SansSerif" SIZE="12"/>
</node>
<node CREATED="1317411421066" ID="ID_850531393" MODIFIED="1319622475795" TEXT="Drop">
<font NAME="SansSerif" SIZE="12"/>
</node>
<node CREATED="1317412576746" ID="ID_478026753" MODIFIED="1319622475794" TEXT="Expand">
<font NAME="SansSerif" SIZE="12"/>
</node>
<node CREATED="1317411424010" ID="ID_289065640" MODIFIED="1319622475794" TEXT="Harass">
<font NAME="SansSerif" SIZE="12"/>
</node>
<node CREATED="1317411416562" ID="ID_1539075835" MODIFIED="1319622475794" TEXT="Move To">
<font NAME="SansSerif" SIZE="12"/>
</node>
<node CREATED="1317412722274" ID="ID_1800313480" MODIFIED="1319622475794" TEXT="Scout">
<font NAME="SansSerif" SIZE="12"/>
</node>
</node>
<node CREATED="1319629447555" ID="ID_154010130" MODIFIED="1319629463563" TEXT="Communication">
<node CREATED="1317412356538" ID="ID_783597189" MODIFIED="1319622475795" TEXT="Ask for or give money">
<font NAME="SansSerif" SIZE="12"/>
</node>
<node CREATED="1319629465441" ID="ID_1223209301" MODIFIED="1319629579147" TEXT="Ping minimap and tell player of enemy actions">
<node CREATED="1319629583931" ID="ID_1797911688" MODIFIED="1319629593887" TEXT="When new bases are found"/>
<node CREATED="1319629594656" ID="ID_1809092044" MODIFIED="1319629617974" TEXT="Seeing a possible attack"/>
</node>
</node>
<node CREATED="1319705607083" ID="ID_1827109253" MODIFIED="1319705749698" TEXT="More than one action at a time?">
<icon BUILTIN="help"/>
<node CREATED="1319705752900" ID="ID_160566868" MODIFIED="1319705961378" VSHIFT="1">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Should the AI be able to scout with an attack unit
    </p>
    <p>
      at the same time as we defend the player?
    </p>
  </body>
</html></richcontent>
<icon BUILTIN="help"/>
</node>
<node CREATED="1319705993878" ID="ID_1865097336" MODIFIED="1319706324210">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      If the player sends a command (move here),
    </p>
    <p>
      should we always obey the command?
    </p>
  </body>
</html></richcontent>
<icon BUILTIN="help"/>
<node CREATED="1319706082812" ID="ID_134436642" MODIFIED="1319706169218">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Maybe no if something important happens
    </p>
    <p>
      (e.g. an attack on the player when he ordered
    </p>
    <p>
      an moveto command to us on the other side of
    </p>
    <p>
      the map). How much initiative?
    </p>
  </body>
</html></richcontent>
<icon BUILTIN="idea"/>
</node>
<node CREATED="1319706172230" ID="ID_1513371403" MODIFIED="1319706268953" TEXT="Yes if the AI has less initiative">
<icon BUILTIN="idea"/>
</node>
</node>
</node>
</node>
</node>
<node COLOR="#000000" CREATED="1317404597672" ID="ID_765469986" MODIFIED="1320086538685" POSITION="left" VGAP="25">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Reaserch fields
    </p>
    <p>
      <font color="#990000">Focus</font>
    </p>
    <p>
      <font color="#996600">Touches </font>
    </p>
    <p>
      <font color="#006600">Uses</font>
    </p>
  </body>
</html></richcontent>
<font BOLD="true" NAME="SansSerif" SIZE="16"/>
<node CREATED="1319622634056" ID="ID_1941502902" MODIFIED="1319709781339" TEXT="Artificial Intelligence" VGAP="26">
<font NAME="SansSerif" SIZE="14"/>
<node CREATED="1319623469000" ID="ID_578070535" MODIFIED="1319629818804" TEXT="Game applications">
<node CREATED="1319705023724" ID="ID_218642254" MODIFIED="1319705025267" TEXT="Animation"/>
<node COLOR="#996600" CREATED="1319630228754" ID="ID_606441037" MODIFIED="1319709891787" TEXT="Behavior">
<edge COLOR="#996600"/>
<font BOLD="true" NAME="SansSerif" SIZE="12"/>
</node>
<node COLOR="#990000" CREATED="1319629681201" ID="ID_683063482" MODIFIED="1319707321398" TEXT="Communication">
<edge COLOR="#990000"/>
<font BOLD="true" NAME="SansSerif" SIZE="12"/>
</node>
<node COLOR="#990000" CREATED="1319629663890" ID="ID_372570435" MODIFIED="1319709891787" TEXT="Cooperation">
<edge COLOR="#990000"/>
<arrowlink DESTINATION="ID_606441037" ENDARROW="Default" ENDINCLINATION="67;3;" ID="Arrow_ID_665956875" STARTARROW="None" STARTINCLINATION="80;-2;"/>
<font BOLD="true" NAME="SansSerif" SIZE="12"/>
</node>
<node CREATED="1319629810372" ID="ID_712338476" MODIFIED="1319629812105" TEXT="Learning"/>
<node CREATED="1319629657350" ID="ID_295611705" MODIFIED="1319629663129" TEXT="Pathfinding"/>
<node COLOR="#996600" CREATED="1319623527123" ID="ID_552072861" MODIFIED="1319707356142" TEXT="Strategy &amp; Planning">
<edge COLOR="#996600"/>
<font BOLD="true" NAME="SansSerif" SIZE="12"/>
</node>
<node CREATED="1319623530275" ID="ID_331760150" MODIFIED="1319623560254" TEXT="Vision"/>
</node>
<node CREATED="1319623474119" ID="ID_422887734" MODIFIED="1319629634487" TEXT="Game AI techniques" VGAP="13">
<node CREATED="1319629734155" ID="ID_323117443" MODIFIED="1319629735467" TEXT="Search">
<node CREATED="1319629835103" ID="ID_1933967591" MODIFIED="1319705374380" TEXT="Optimal &quot;path&quot;"/>
<node CREATED="1319705347485" ID="ID_1012585460" MODIFIED="1319705371415" TEXT="Near optimal &quot;path&quot;" VGAP="15">
<node CREATED="1319629983229" ID="ID_157462529" MODIFIED="1319629990500" TEXT="Pathfinding">
<node CREATED="1319629991136" ID="ID_1529947842" MODIFIED="1319629993440" TEXT="Waypoints"/>
<node CREATED="1319629995710" ID="ID_1852460591" MODIFIED="1319630001917" TEXT="Potential fields"/>
</node>
<node COLOR="#006600" CREATED="1319705415834" ID="ID_133403667" MODIFIED="1319708502994" TEXT="Planning Goals">
<edge COLOR="#006600"/>
<arrowlink DESTINATION="ID_1981216470" ENDARROW="Default" ENDINCLINATION="175;3;" ID="Arrow_ID_1527417384" STARTARROW="None" STARTINCLINATION="133;121;"/>
<font BOLD="true" NAME="SansSerif" SIZE="12"/>
</node>
</node>
</node>
<node CREATED="1319629742663" ID="ID_1639468786" MODIFIED="1319629846376" TEXT="Machine learning">
<node CREATED="1319629857132" ID="ID_14906997" MODIFIED="1319629860327" TEXT="Inductive learning"/>
<node CREATED="1319629862455" ID="ID_1519774804" MODIFIED="1319629868348" TEXT="Artificial neural networks"/>
<node CREATED="1319629870134" ID="ID_366972649" MODIFIED="1319629873457" TEXT="Genetic algorithms"/>
<node CREATED="1319629884244" ID="ID_1173854979" MODIFIED="1319629957095" TEXT="Decision trees"/>
</node>
<node CREATED="1319630215897" ID="ID_143770545" MODIFIED="1319630219074" TEXT="State machines">
<node CREATED="1319705292412" ID="ID_1311702317" MODIFIED="1319705298668" TEXT="Hierachical state machines"/>
</node>
<node COLOR="#006600" CREATED="1319630016856" ID="ID_1981216470" MODIFIED="1319708502994" TEXT="Goal-oriented actions">
<edge COLOR="#006600"/>
<font BOLD="true" NAME="SansSerif" SIZE="12"/>
</node>
</node>
<node CREATED="1319623480953" ID="ID_941113370" MODIFIED="1319630030754" TEXT="Knowledge representataion">
<node CREATED="1319629698786" ID="ID_91019716" MODIFIED="1319629703998" TEXT="Semantic networks"/>
<node COLOR="#006600" CREATED="1319629704999" ID="ID_376409335" MODIFIED="1319708462426" TEXT="Rule-based systems">
<edge COLOR="#006600"/>
<arrowlink DESTINATION="ID_133403667" ENDARROW="Default" ENDINCLINATION="169;0;" ID="Arrow_ID_48761808" STARTARROW="None" STARTINCLINATION="229;0;"/>
<font BOLD="true" NAME="SansSerif" SIZE="12"/>
</node>
<node CREATED="1319629708741" ID="ID_1143540495" MODIFIED="1319629717490" TEXT="Neural networks"/>
<node CREATED="1319629718039" ID="ID_1034036731" MODIFIED="1319629720589" TEXT="Frames"/>
<node CREATED="1319629720914" ID="ID_1287821470" MODIFIED="1319629725254" TEXT="Predicate logic"/>
</node>
</node>
</node>
<node CREATED="1319716698544" ID="ID_1841977840" MODIFIED="1319717690197" POSITION="left" TEXT="Possible References" VGAP="13">
<node CREATED="1319717777777" ID="ID_1987387968" MODIFIED="1319717780617" TEXT="Reference Styles">
<node COLOR="#003366" CREATED="1319716783825" ID="ID_581790588" MODIFIED="1319717914084" TEXT="Games">
<edge COLOR="#003366"/>
<font BOLD="true" NAME="SansSerif" SIZE="12"/>
<icon BUILTIN="clanbomber"/>
</node>
<node COLOR="#660000" CREATED="1319716789005" ID="ID_1293287607" MODIFIED="1319717982724" TEXT="Papers">
<edge COLOR="#660000"/>
<font BOLD="true" NAME="SansSerif" SIZE="12"/>
<icon BUILTIN="edit"/>
</node>
<node COLOR="#006600" CREATED="1319716818953" ID="ID_1702261938" MODIFIED="1319717946659" TEXT="Books/Articles">
<edge COLOR="#006600"/>
<font BOLD="true" NAME="SansSerif" SIZE="12"/>
<icon BUILTIN="pencil"/>
</node>
<node COLOR="#009999" CREATED="1319716831428" ID="ID_1352660594" MODIFIED="1319717958072" TEXT="Theses">
<edge COLOR="#009999"/>
<font BOLD="true" NAME="SansSerif" SIZE="12"/>
<icon BUILTIN="group"/>
</node>
</node>
<node CREATED="1319717771306" ID="ID_753767364" MODIFIED="1319718171845" TEXT="References">
<node CREATED="1319716851237" ID="ID_1378489456" MODIFIED="1319716879740" TEXT="Voice Control">
<node CREATED="1319718104484" ID="ID_313905624" MODIFIED="1319718107501" TEXT="RTT">
<node CREATED="1319716881319" ID="ID_1395067353" MODIFIED="1319718219453" TEXT="Endwar, can use vioce for input">
<icon BUILTIN="clanbomber"/>
</node>
</node>
<node COLOR="#000000" CREATED="1319721114139" ID="ID_1631201965" MODIFIED="1319724136894" TEXT="TPS">
<node CREATED="1319721130827" ID="ID_1355405334" MODIFIED="1319721157574" TEXT="Mass Effect 3, use voice to order commands">
<icon BUILTIN="clanbomber"/>
</node>
</node>
</node>
<node CREATED="1319718018902" ID="ID_288167213" MODIFIED="1319718024186" TEXT="Cooperation">
<node CREATED="1319718610710" ID="ID_997572589" MODIFIED="1319720673998" TEXT="FPS">
<node CREATED="1319718615441" ID="ID_1924489016" MODIFIED="1319721410453" TEXT="Being a Better Buddy: Interpreting the Player&apos;s Behavior">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      AI Game Programming Wisdom 3: 6.5
    </p>
    <p>
      Tags: article, fps, player behavior
    </p>
  </body>
</html></richcontent>
<icon BUILTIN="pencil"/>
</node>
</node>
<node CREATED="1319721208656" ID="ID_150573860" MODIFIED="1319721209846" TEXT="RTS">
<node CREATED="1319721210797" ID="ID_1885194669" MODIFIED="1320139944681" TEXT="Player Adaptive Cooperative Artificial Intelligence for RTS Games">
<icon BUILTIN="group"/>
<icon BUILTIN="yes"/>
</node>
</node>
<node CREATED="1319721860683" ID="ID_1567055676" MODIFIED="1320139944680" TEXT="Real-time team-mate AI in Games">
<icon BUILTIN="edit"/>
<icon BUILTIN="yes"/>
</node>
<node CREATED="1319723006777" ID="ID_365699139" MODIFIED="1320139944680" TEXT="Personality-based Adaption for Teamwork in Game Agents">
<icon BUILTIN="edit"/>
<icon BUILTIN="yes"/>
</node>
</node>
<node CREATED="1319720921245" ID="ID_234333388" MODIFIED="1319720923447" TEXT="Communication">
<node CREATED="1319720944641" ID="ID_1106915023" MODIFIED="1319720946227" TEXT="FPS">
<node CREATED="1319720675698" ID="ID_1335171590" MODIFIED="1320139944680" TEXT="Human, all too non-Human: Coop AI and the Conversation of Action">
<icon BUILTIN="edit"/>
<icon BUILTIN="yes"/>
</node>
</node>
<node CREATED="1319721268342" FOLDED="true" ID="ID_1221689759" MODIFIED="1319721805494" TEXT="The Design of Future Things">
<icon BUILTIN="pencil"/>
<node CREATED="1319721425538" ID="ID_687998661" MODIFIED="1319721586802">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p style="text-align: right">
      The AI should convey it's intensions to the user
    </p>
    <p style="text-align: right">
      (Don't keep the user in the dark)
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1319721642104" ID="ID_491163781" MODIFIED="1319721650766" TEXT="Design Rules p.193"/>
</node>
</node>
<node CREATED="1319718158903" ID="ID_768568334" MODIFIED="1319718730198" TEXT="Player Behavior Analysis">
<node CREATED="1319718198951" ID="ID_1393598763" MODIFIED="1319718479657" TEXT="Player Modeling for Adaptive Games">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      AI Game Programming Wisdom 2: 10.1
    </p>
  </body>
</html></richcontent>
<icon BUILTIN="pencil"/>
</node>
<node CREATED="1319721068919" ID="ID_1557383836" MODIFIED="1320139944679" TEXT="AI for Dynamic Team-mate Adaption in Games">
<icon BUILTIN="edit"/>
<icon BUILTIN="yes"/>
</node>
</node>
<node CREATED="1319721920925" ID="ID_277078248" MODIFIED="1319721927279" TEXT="Design"/>
</node>
<node CREATED="1319717015869" ID="ID_404252165" MODIFIED="1319717021043" TEXT="Noteworthy quotes"/>
</node>
</node>
</map>
