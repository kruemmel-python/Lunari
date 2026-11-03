# Evolution und Kommunikation

Pro Generation werden Kandidaten mit separaten Agentenlexika getestet. Im Training erhält der Empfänger erst nach einem Kommunikationsversuch Rückmeldung über die beteiligten Atomzuordnungen. Der finale Kandidat trainiert eine persistente Population mit getrennten Reverse-Lexika und eigenen Erfolgs-/Fehlerhistorien. Im Evaluationsteil wird keine Rückmeldung gegeben; er verwendet bewusst Kombinationen mit Relation `if`, Quantität `3`, Modalität `required` und berichteter Aussage, die nicht als solche im Trainingsgenerator vorkommen.

Eine Mutation ersetzt entweder genau eine lautliche Realisierung oder vertauscht zwei Felder der expliziten Satzreihenfolge. Tokenidentitäten und strukturierte Bedeutungen bleiben dabei stabil; die durch die Grammatikmutation veränderte Nachricht wird vom Empfänger nur über seine private Reverse-Lexikon-Zuordnung gelernt und dekodiert. Das trennt kontrolliert phonologische und syntaktische Variation. Die Bewertung misst weiterhin unabhängige, zuvor nicht verwendete Bedeutungszusammensetzungen.

Die Grammatikreihenfolge wird zusätzlich als normierte Inversionskomplexität bewertet: `1` steht für die kanonische, vollständig regelmäßige Feldfolge; jede Vertauschung erhöht den Wert bis höchstens `2`. Diese Komplexität wird als zu minimierendes Ziel sowohl in der Pareto-Dominanz als auch in der GPU-/CPU-identischen Kandidatenreihung berücksichtigt.

Der Bericht bezeichnet die Evaluierung ausdrücklich als simulierte Agentenleistung. Es gibt keine Ableitung realer menschlicher Lernbarkeit ohne Versuchspersonen.
