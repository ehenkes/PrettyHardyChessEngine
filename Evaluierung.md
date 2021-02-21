# PrettyHardyChessEngine
Weiterentwicklung der Chess Engine von Bill Jordan

21.02.2020:
Durch Erhöhung des Wertes für den Läufer von 3.00 auf 3.25 und durch Schaffung eines Bonus für das Läuferpaar konnte die Auswahl des besten Zuges verändert werden.

Bei der Position
r1b2rk1/ppppqppp/2n2n2/6N1/1bBPp3/2N1P3/PPP2PPP/R1BQ1RK1 w - - 6 9
mit Weiß am Zug wurde nicht mehr der Patzer Lxf7+ gespielt, sondern Sxf7. Noch nicht der gemäß SF12 beste Zug d5, aber ein deutlicher Fortschritt.

Bei der Position
3r1r1k/6pp/ppq2b2/2pnp3/4Q3/N1P5/PP1B1PPP/3RR1K1 w - - 0 26
schwankt die Engine zwischen dem korrektem Zug Sc4 und dem Patzer g4.
Tiefe 12: g4 (falsch)
Tiefe 13: Sc4 (sehr gut)
Tiefe 14: Lc1 (gut)
Diese Position ist ein Beispiel, bei dem sich Halbzugtiefe auswirkt und daher lohnen kann.


Bei der Position
2b1r1k1/p1p1q3/1p1p2Pp/n2P1R2/4n1Q1/4P3/PBP3P1/4R1K1 w - - 0 26
zieht die Engine nach wie vor den Patzer g7.
Besser ist Dh3, Dh5 oder Df4. Dann folgt ...Lxf5 und Dxh6! Das Vorrücken des g-Bauern zerstört diese taktischen Drohungen komplett und verliert.

Bei der Position
4r1k1/p1p1q2p/bp1p2p1/n2P1R1P/4n1Q1/4P3/PBP3P1/4R1K1 b - - 0 24
bleibt die Engine ebenfalls noch bei dem Fehler/Patzer ...Lc8. Richtig wäre ...g5. Besser ist auch ...Sc4.

Bei der Position
6k1/6pp/p1n5/1pp1p3/6Pb/2P1Nr2/PP3P1P/2BR2K1 w - - 2 34
erfolgt ab Tiefe 13 und bei Tiefe 14 der beste Zug Sf5 (vorher Tf6).

Daher ist eine Beschleunigung der Suche/Bewertung wichtig, damit man möglichst mindestens 14 Halbzüge spielen kann.




