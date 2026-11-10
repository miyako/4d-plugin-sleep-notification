# Sleep Notification ownership audit

## Immutable evidence

Baseline: `8e6ab6e9cd99db79554dc129b7db196f514497d4` (default branch).
Canonical guidance: `miyako/skills`, `4d-skills/skills/4dplugin/SKILL.md`,
revision `e6b492fcb81e5e1a7f7db723594b420e63bf6eb9`, read through the
immutable GitHub contents API, without another checkout.

The bundled API header identifies itself as v13; its implementation says
v14. These labels do not establish an upstream SDK release. The complete
engine implementation and ABI headers remain unchanged. The regression runner
checks their exact Git blob IDs before compiling:

| File in `4D Plugin API/` | Git blob |
|---|---|
| `4DPluginAPI.c` | `478081f0238f6bc5393222f574bc2656f010b67b` |
| `4DPluginAPI.h` | `9054b45ffc7ff6f5b8d57529a3788a55d977b314` |
| `4DPluginAPI.def` | `996ecc4ab11ad165de9192eba2e7478db141c757` |
| `EntryPoints.h` | `730f2f77ff2713fb2fe4b7d6562baeedf5f664cb` |
| `Flags.h` | `10a83185333dac06a62dda61b9f9428243d1e2ee` |
| `PrivateTypes.h` | `77053dc0c0a3718d2d3b6f158371a6af78c6ae66` |
| `PublicTypes.h` | `2bb392e73e9c556c274fe0a75b3a6b83e310b77c` |

Prior art was read as immutable files from `miyako/4d-plugin-script` at
`c13f12088f8c88b2f0fa6f5e79104759707ec7e2`: the ownership host, runner,
platform redirection header, and picture wrapper. This target's complete SDK,
dispatcher, platform boundaries, fixtures and results are checked independently.
In particular its `4DPluginAPI.c` blob differs from that prior art.

## Complete tracked native and build inventory

At baseline there are 18 native translation units: the dispatcher, complete
SDK implementation, and 16 wrapper implementations. Both the Xcode Sources
phase and the VCXPROJ list all 18. Each wrapper below has a tracked `.cpp`
and `.h`; none was excluded because it is dormant.

| Files / classes | Actual storage and ownership surface | Reachability |
|---|---|---|
| `4DPlugin.cpp`, `4DPlugin.h` | `Listener`, notification queues, persistent `SN::LISTENER_METHOD`, scalar setters/returns, callback longint argument and owning method return; power XML into `ARRAY_TEXT` | Production macOS dispatcher and worker entry points |
| `Classes/C_TEXT` | Owns heap `CUTF16String`; copies borrowed scalar inputs; content-setter output; Foundation path/string conversions; already noncopyable | Production commands, UUID/process-name and SDK process-info helper; also used by dormant wrappers |
| `Classes/ARRAY_TEXT` | Owns heap vector of value strings; copies borrowed input elements; creates distinct owned `PA_Unistring` per exported element, including zero; Foundation conversion/path helpers | Production power-information array |
| `Classes/C_LONGINT` | Value-only integer, parameter/return forwarding | Production command results |
| `Classes/C_PICTURE` (`CPicture`, `C_PICTURE`) | Retained `PA_Picture`, owned `CBytes`, command path/format strings, owned argument and result variables, metadata strings with unresolved host contract | Compiled dormant; no dispatcher caller |
| `Classes/C_POINTER` | Borrowed pointer and cached kinds; constructs variable/field reference metadata | Compiled dormant; no dispatcher caller |
| `Classes/C_BLOB` (`CBytes`, `C_BLOB`) | `CBytes` holds vector by value; `C_BLOB` owns heap `CBytes`; hex/base64 use independent `C_TEXT` content; blob parameter/return handle ownership | Compiled dormant, except internal `CPicture` byte storage |
| `Classes/ARRAY_BOOLEAN` | Owns heap `vector<bool>`; boolean handle import/export | Compiled dormant |
| `Classes/ARRAY_DATE` | Owns heap `vector<C_DATE>`; elements are copyable value-only dates | Compiled dormant |
| `Classes/ARRAY_INTEGER` | Owns heap short vector; integer handle import/export | Compiled dormant |
| `Classes/ARRAY_LONGINT` | Owns heap int vector; longint handle import/export | Compiled dormant |
| `Classes/ARRAY_REAL` | Owns heap double vector; real handle import/export | Compiled dormant |
| `Classes/ARRAY_TIME` | Owns heap int vector; time handle import/export | Compiled dormant |
| `Classes/C_DATE`, `C_INTEGER`, `C_REAL`, `C_TIME` | Value-only fields and parameter/return accessors; no owned string allocation | Compiled dormant; `C_DATE` also used by date array |
| `Classes/C_TYPES.h` | UTF-8/UTF-16 value-string, vector and parameter typedefs; Foundation/Windows includes | Shared header |
| `4DPluginAPI.c/.h` | Complete API forwarding, direct shallow setters, array resize and accepted-transfer predicates; SDK-internal text/process helper | Compiled complete, not replaced with fragments |
| `PublicTypes.h`, `PrivateTypes.h`, `Flags.h`, `EntryPoints.h` | Packed ABI, engine selectors and platform selection | Protected unchanged headers |
| `4DPluginAPI.def` | Single Windows export `FourDPackex`; SDK constructs `PluginBlock` then calls actual `PluginMain` | Export declaration unchanged |
| `4D Plugin_Prefix.pch` | Carbon include | Xcode prefix unchanged |
| New `Classes/CommandArguments.h` | Noncopyable scoped owners, undefined initialization, checked command call and successful result cleanup | Included by picture implementation; no project membership change needed |

Also inspected: `Sleep Notification .xcodeproj/project.pbxproj`, scheme and
workspace settings, `Sleep Notification .vcxproj` and `.filters`, both source
and bundled manifests, constants/docs, both existing workflow files, all
`.github/scripts/*.py`, the bump pre-push hook, and the existing lifecycle
tests. Xcode phases are Headers, Resources, CopyFiles, Sources, Frameworks
and Rez; no shell-script phase. No project build was run. Release workflow
`Build` performs signing, notarization, packaging and publication, but is
manual-only; `Bump version` is also manual-only. Neither is invoked or changed.

The root and bundled manifests agree:

| Selector | Signature | String ownership surface |
|---|---|---|
| 1 | `SN Set sleep method(&T):L` | Borrowed input copied into persistent value string |
| 2 | `SN Get sleep method:T` | Content setter copies persistent UTF-16 into return |
| 3 | `Get battery warning level:L` | No text |
| 4 | `GET POWER INFORMATION(&Y)` | Independent owned array elements |

Tracked bundle binaries, resources and 4D database files are not audit
execution inputs and remain untouched. No additional native source copies,
JSON/object/collection wrappers, or property/collection getter call sites
were found in the tracked index. That absence is inventory evidence, not a
proof that an arbitrary host operation is safe. Objects/collections are
reported as **n/a**, not simulated as nonexistent owned references.

## Exact ownership contracts and fixes

SDK implementation locations refer to the immutable baseline.

- `PA_SetStringInArray` (4444-4460) disposes the old value and shallow assigns
  the supplied descriptor. It accepts Unicode kind, non-null data, non-null
  descriptor and `0 <= index <= count`, including zero. Rejection transfers
  nothing. `ARRAY_TEXT::toParamAtIndex` already allocates each element
  separately and does not dispose after transfer: **no array rewrite**.
  Import uses independent `CUTF16String` copies before replacement or clear.
- `PA_SetStringVariable` (3826-3831) shallow assigns, whereas
  `PA_CreateVariable(eVK_Unistring)` (3630-3634) can create an allocated empty
  owner. Picture path/format arguments now start Undefined, take ownership
  exactly once and are cleared exactly once; the extra path dispose is gone.
- `PA_GetPictureVariable` (3729-3737) explicitly returns a borrowed picture.
  Picture command output is retained before scoped argument cleanup.
  `PA_DuplicatePicture(..., 1)` retains (447-454). `setBytes` releases the
  temporary creation reference after `setPicture` retains it.
- Picture commands now clean up successful owning return variables, including
  string/picture values unrelated to the requested output. Failure is checked
  before return inspection, adoption or cleanup. Command handle failure
  (5257-5296) can return raw kind zero; `PA_GetVariableKind` normalizes this
  as Unicode, so a kind check is not a valid success test.
- `createCopy`'s engine converter forwards byte-sized input/output buffers
  and returns output **bytes**. The wrapper divides by `sizeof(PA_Unichar)`
  before constructing a length-bearing UTF-16 string. It checks conversion
  failure before reading the output buffer.
- `PA_GetPointerValue` (2035-2052) returns an owning value, not reference
  metadata. The wrapper no longer calls it or interprets its value union as
  metadata. `PA_SetVariableOrFieldReference` (3923-3948) is the exact bundled
  constructor: scope mapping `{26,9,35,13}`, filler one, copied name/tag/index,
  scope in definition type; fields use `PA_SetTableFieldVariable`.
  Invalid scope is rejected before that constructor's four-element lookup.
- The seven array wrappers, `C_BLOB`, `CPicture` and `C_PICTURE` allocate raw
  owned pointers and delete them in destructors. Their implicit copy
  constructor/assignment exposed double ownership. Both operations are
  deleted, following the existing `C_TEXT` pattern. No production wrapper
  copies were found. This is a **latent capability fix**, not a reported
  production copy crash. Value-only `C_DATE` and `CBytes` remain copyable.
- The production worker's synchronous callback receives only a longint
  argument; it does not share an array-owned string. However,
  `PA_ExecuteMethodByID` (5189-5225) returns a separate owning variable copied
  out before the SDK frees its parameter handle. Ignoring a successful text
  or picture result leaked it. The worker now handles that owner separately
  from the callback argument and never inspects an unspecified failed return.
  It captures the SDK error immediately, clears the argument, then clears
  only a successful return. A failed callback continues with the pre-existing
  behavior; no exception or new error-reporting channel is introduced.

## Isolated regression procedure

Run on macOS:

```sh
python3 .github/tests/test_ownership.py -v
python3 -m unittest discover -s .github/tests -p test_lifecycle.py -v
```

The runner compiles the **actual** dispatcher, all 16 actual wrappers, and
complete unchanged bundled SDK into a temporary unsigned executable.
`ownership_platform.h` is force-included before every translation unit.
It redirects NSWorkspace, NSNotificationCenter, NSUUID, NSImage, NSURL,
IOPS power/battery functions, property-list generation and filesystem URL
conversion to synthetic stubs. The final executable's undefined imports
are checked for real power, notification, process, filesystem and UI
boundaries. The host rejects every unexpected engine selector. Stub process
functions record identities; they launch no OS/4D processes. Observer
registration/removal is against an in-memory synthetic center only.

Assertions cover:

- ASCII, BMP, supplementary surrogate pairs and empty text; allocated and
  null empty string engines; exact lengths/content, terminators, buffer
  capacities and bytes versus UTF-16 units.
- Array element zero, distinct owners, borrower-to-value copies, allocator
  churn, repeated replacement, shrink/clear, Undefined destination, invalid
  parameter index and all SDK setter rejection predicates.
- All picture helper operations, independent output retains, owning command
  returns, poisoned failed returns, command handle failure, conversion
  failure and failed picture creation without adoption.
- Pointer reference metadata in interpreted/compiled parameter layouts,
  fields, null/uninitialized pointers and invalid scope, without fetching an
  owning value.
- Named copy-constructor/assignment executions against baseline and
  noncopyable fixed traits for each of the ten affected owner classes.
- Actual macOS dispatch of scalar/power output and synthetic listener
  setup/get/notification/callback/stop/termination, successful Undefined,
  text and picture callback returns, plus poisoned failure returns and
  command-handle failure.

ASan and UBSan are enabled; only alignment checks are disabled because this
unchanged SDK exposes packed ABI structures. LeakSanitizer is disabled on
macOS; explicit allocation maps require zero remaining string, picture and
handle owners in every leaf process. Foundation autorelease fixtures drain
inside a local pool; real platform operations are never requested.

The identical final harness must also be run against `git archive` of the
immutable baseline (native sources only), using `OWNERSHIP_SOURCE_ROOT`
to select it. Test and platform files remain the same for both executions.
Detailed local logs are retained in the session artifacts, not the repository.

Local execution of that identical final harness produced:

| Named regression group | Baseline passing / total | Fixed passing / total |
|---|---:|---:|
| Actual listener callback owners, failed returns and continuation | 6 / 18 | 18 / 18 |
| Actual scalar/power dispatcher controls | 2 / 2 | 2 / 2 |
| Array transfer/replacement/rejection controls | 2 / 2 | 2 / 2 |
| Baseline copy-constructor/assignment sanitizer cases | 0 / 20 | 20 / 20 |
| Picture command/creation/conversion failures | 0 / 12 | 12 / 12 |
| Owner copy traits and value-type controls | 2 / 12 | 12 / 12 |
| Exact path/format text and empty-owner variants | 0 / 16 | 16 / 16 |
| Picture creation, borrowing and handoffs | 0 / 4 | 4 / 4 |
| Pointer field metadata control | 1 / 1 | 1 / 1 |
| Pointer null/invalid-scope regression | 0 / 1 | 1 / 1 |
| Interpreted/compiled pointer metadata | 0 / 2 | 2 / 2 |
| Scalar content-setter controls | 8 / 8 | 8 / 8 |
| Exact SDK failure-predicate controls | 1 / 1 | 1 / 1 |
| Successful owning picture-command returns | 0 / 15 | 15 / 15 |
| **Total leaf cases** | **22 / 114** | **114 / 114** |

Baseline failures include ASan heap-use-after-free in owning wrapper copies,
ASan heap-buffer-overflow in `createCopy`, duplicate path-string disposal,
explicit nonzero string/picture balances, and incorrect pointer metadata.
The same fixed executable passes with zero tracked owners after each case.
Existing offline lifecycle tests also pass: **53 / 53**.

## Explicit limits and deferrals

- **Not a real 4D host test.** No Windows 4D crash has been reproduced.
  The Objective-C macOS dispatcher is exercised, not mislabeled as Windows
  branch coverage. The legacy Windows project lists that macOS-only source
  and the committed prebuilt `.4DX` is not rebuilt or certified.
- **Not OOM certification.** `PA_ResizeArray` (3973-4134) changes logical count
  after `PA_SetHandleSize` fails. A named test proves logical/physical
  divergence and repairs only the synthetic descriptor for cleanup.
  Production `ARRAY_TEXT` has no proven transactional failure-preservation
  boundary in its void exporter. This SDK limitation is deferred, not hidden
  behind silent empty-success output or an SDK rewrite.
- `PA_CreateUnistring` does not publish an error through `sErrorCode`.
  Allocation/constructor failure and overflow in legacy wrappers are not
  certified by successful allocations. Command-failure fixtures preserve
  valid argument owners while poisoning the unspecified return; they do not
  prove arbitrary host corruption of argument descriptors is recoverable.
- Successful `PA_GetPictureData` metadata-string ownership is not stated by
  this exact header/implementation. `_GetCount`/`loadImageOfType` therefore
  receive only a poisoned **failed** metadata result in the host, checked
  before use. Successful metadata ownership/disposal and real image codecs
  remain deferred; no speculative metadata dispose was introduced.
- Existing broad `PluginMain` exception swallowing, malformed-input handling,
  unrelated blob codec behavior, scheduling semantics and platform API
  failures are not broadened into a general rewrite.
- Callback failures have no existing standard reporting path in this source.
  This reporting gap is explicitly deferred unchanged behavior, not a
  success fallback. The SDK error is captured only to protect return cleanup;
  no claim is made that SDK last-error survives argument cleanup. Synthetic
  failed callbacks must leave their poisoned return untouched, throw no
  exception, clean their argument, and deliver the next event normally.
- No signing, packaging, notarization, release, tag, manual dispatch, install,
  system discovery, real notification/observer, private-data, power, sleep or
  wake operation is part of these tests. The new path-filtered ownership CI
  compiles this isolated host only; it does not invoke a project build.
