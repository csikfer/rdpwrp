rdpwrp
======

Egy GUI az RDP kliens indításához LTSP környezetben

A program megírását az indololta, hogy a Windows terminálszerverek új 2012-es verziójával nem műödtek a terminál kliensek.
A régi 2008-as windows-al is csak akkor, ha ki volt kapcolva a session broker.
A 2012-es terminál szerver esetén ha több terminál szerver van, csak úgy működik a freerdp, ha paraméterként meg van adva a
felhasználónév és jelszó, amit az LTSP jelenleg nem támogat.

A program installálása:
A kliens kiosk módban működik, és a futtatandó parancs (a "böngésző") az rdpwrp lessz. Az ltsp.cfg -ben:

SCREEN_07=kiosk rdpwrp

A programot elhelyezhetjük a kliens környezet /usr/local/bin/ könyvtárában. A program három adatfájlt fog keresni az aktuális
könyvtában a .rdpwrp mappában. Mivel az aktuális könyvtár a home, ezért ezt a mappát, és benne a három adatfájlt a home könyvtár
sablon könyvtárába kell elhelyezni, ami a kliens környezetben a /usr/local/share/ltspkiosk/home könyvtár.
A .rdpwrp mappában elhelyezendő fájlok:

A domains fájl

A windows tartományok és terminál szerverek listája. Legalább egy tartományt meg kell adni, és minden tartományban lennie kell egy
szerver névnek. A tartomány nevek a sor elején kezdődnek, a szerver nevek pedig alattuk legylább egy bevezető szóközzel.
Az üres sorok figyelmenkívül lesznek hagyva.

A commands fájl

A rdpwrp által hívható parancsokat definiálja. Minden sor két mezőből áll, ahhol az elválasztó karakter a '|'.
Az első mező a parancs megjelenített neve, vagy a '!' ill. '!!', a második mező a parancs.
Két kitüntetett parancs van a '!', ami az xfreerdp hívását definiálja. Ebben a parancsban négy szimbólikus mező
van, ahhol a %1 a tartomány név, %2 a szerver neve, %3 a felhasználó név, és a %4 a jelszó.
A '!!' kitüntetett parancs pedig a klienst kikapcsoló parancs sor, mely akkor hajtódik végre, ha hosszabb ideig
tétlen a program. Ha ezt a parancsot meg akarjuk adni a menüben is, akkor a parancs mezőben a '!!'-val hivatkozhatunk rá.
egy példa:

!|/usr/bin/xfreerdp --ignore-certificate -f -z -a 24 --plugin rdpsnd --plugin rdpdr --data disk:USB:/media/root -- -d %1 -u %3 -p %4 %2
!!|/usr/bin/sudo /sbin/poweroff
Terminál kikapcsolása|!!
Segítség|/usr/bin/chromium-browser http://help.domain.local/help
Böngészó indítása|/usr/bin/chromium-browser
Parancs sor|/usr/bin/xterm

Alapértelmezetten a kioskuser felhasználónak nem lessz joga elindítani a poweroff parancsot, ezért a kliens környezetben
a /etc/sudoers fájlt ki kell egészíteni a következő sorral:

ltspkiosk       ALL=(root) NOPASSWD: /sbin/poweroff

A styles fájl (opcionális)

A megjelenést kehet módosítani. pl.:

background-color: darkseagreen

