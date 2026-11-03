# CORTEX – autonomous lexicon evolution

## Evidence model

Every persisted concept is explicitly classified. `external-attested` means an
imported source with an external identity and provenance; `simulated-relation`
is a transparent composition derived from a stored relation; `agent-hypothesis`
is local research material. These classes are never merged in counters.

Language state is likewise separate: `realised` means the engine assigned a
regular form; `communicatively-validated` means independent sender/receiver
maps reconstructed an unseen compositional family. The two existing persistent
agent records receive the corresponding success/failure counters; `rejected` is retained as a
failure record. An imported root is not automatically communication validated.

The current language-formation rule is
`morphology-v4-neu-lunari-compact-cvc;relation-compound-v2`. It uses compact
one-to-three-syllable `V`/`CV`/`CVC` forms, with only a restricted set of
coda-onset pairs, and derives relational families by ordered composition.
This intentionally avoids a uniform four-syllable CV cadence while retaining a
large deterministic form space. The rule version is shown in every lexicon
trace and is distinct from a Wikidata entity/source revision.

## Intake and continuation

The canonical UTF-8 TSV schema is:

`external_id, label, part_of_speech, semantics, relations, source, provenance, source_revision, evidence_class`

`--import-batch state.leng source.tsv 1000` stores a source fingerprint and the
next source line in the state. Re-running it after interruption resumes that
same source; a changed input fingerprint starts a new documented import. The
legacy six-column TSV schema remains supported.

For large real sources, use `scripts/convert_wikidata_dump.ps1` with an
officially obtained local Wikidata JSON/JSON.GZ dump. It streams the dump,
writes bounded canonical rows, retains Q-identities, source URL, dump name,
revision and Wikidata's CC0 provenance. It deliberately performs no public
endpoint mass download.

For controlled online intake, `scripts/collect_wikidata_range.ps1` reads the
official `wbgetentities` endpoint in batches of at most 50 IDs. It defaults to
one request every 5 seconds, records a JSON checkpoint after every request,
uses `languagefallback=1`, applies exponential backoff after rate limits,
stops after repeated transport errors, and writes
only rows with both a label and a description. It is an intentionally slow,
auditable alternative to public query-endpoint scraping.

## Controlled research loop

1. Create or load a generation state.
2. Run bounded `--autonomous-step state.leng source.tsv 1000` calls.
3. Persist after every call (the command does this itself), inspect
   `--lexicon-status`, and use `--search-lexicon` for provenance-level review.
4. Continue until the status says `import-status=complete`; export the state
   and its evidence counters.

`scripts/run_autonomous_lexicon.ps1` is the persistent end-to-end driver: it
creates a fresh state if needed, resumes the controlled collector, then commits
each native import batch before proceeding. Closing it is a pause, not loss of
research state.

At 1,000, 10,000 and 100,000 actual accepted source rows it creates separately
replayable `wikidata-milestone-<N>.leng` states from exactly the first `N`
external rows. These are fresh imports, not filtered copies of the final graph.

The first actual milestone has been verified: `wikidata-milestone-1000.leng`
and its retained `wikidata-milestone-1000.tsv` contain 1,000 qualified external
Wikidata concepts, 313 explicit relation-family entries, and 320 independently
communicatively validated compositions. The 10,000 and 100,000 artifacts are
created only when their sources actually exist.

The 10,000 milestone is also verified: `wikidata-milestone-10000.leng` and
`wikidata-milestone-10000.tsv` contain 10,000 qualified external Wikidata
concepts, 4,495 relations-family entries and 4,502 independently validated
compositions. The same gate remains mandatory for the final 100,000 milestone.

Use `scripts/verify_lexicon_milestone.ps1 -State <state> -ExpectedExternal N`
as the machine-checkable acceptance gate. It refuses a milestone without `N`
stored and qualified external concepts, generated forms, and at least one
independently validated concept-family.

## Current measured scale result

On 2026-11-01 the native KR Windows release processed a 100,000-row
**NON-EVIDENCE** capacity fixture. It stored 100,000 source rows and 99,999
relations-family entries (200,008 concepts including seeds) in a 64,658,340
byte state. Fresh load plus indexed search took 4.65 seconds; intake took 7.83
seconds. This proves storage, checkpoint representation, graph formation and
search scale only. It is **not** reported as an external Wikidata corpus or as
100,000 qualified concepts.

The remaining acceptance condition is an actual 100,000-row local official
Wikidata dump conversion and intake, followed by its separately reported
external-attested, realised and communicatively-validated counts. No such dump
is currently present in the workspace, so the system cannot honestly claim the
final external-data milestone yet.
