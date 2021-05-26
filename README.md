# SunsetSwitch

Ovládání relé (např. světel) pomocí Wemos Mini na základě získáného času z NTP a následného výpočtu soumraku dle geolokace


Nejdříve proběhne připojení k WiFi

Poté proběhne získání času z některého NTP (Network Time Protocol)

Na základě získaného času bude vypočteno datum (kvůli přibližnému určení letního času)

Zjistíme časy občanského soumraku východu / západu slunce

Pak už máme čas soumraku, aktuální čas a můžeme tedy vyfiltrovat dle našich požadavků a sepnout/vypnout relé
