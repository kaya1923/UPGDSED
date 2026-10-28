# Curated Windows symbols

These PDBs were downloaded from the Microsoft public symbol service while
analyzing the Windows 10 `10.0.19041.7802` target and the recorded crash.
Keeping them here makes the offset and dump analysis reproducible without
depending on a warm local symbol cache.

| Symbol | GUID/age cache key | Size | SHA-256 |
| --- | --- | ---: | --- |
| `ntkrnlmp.pdb` | `91A0697CBDF1F6C1B8D282E109E671E61` | 8,571,904 | `C2EE8D225300ADC898797FC5E9ECA2F638B5D2CE672E1F5E8B314F68DD8601D8` |
| `ndis.pdb` | `F108524A6FEA4F42F278709EDD32932E1` | 2,845,696 | `3EA641234132FC879B4F8B0A28356E29E2FF9E52D1799E9375AD2A52DB623103` |
| `netio.pdb` | `B7B2D009AD14CE721FA2BDF385145B5A1` | 839,680 | `57095ED20DDF1428C130310CBAA3E9F78F23433D909FF3E58266A08B514138A8` |
| `nsiproxy.pdb` | `A4AFC5F1D0E6C66666DC2D7D428523F71` | 126,976 | `B506F63DA1601E0D9A006A7322380C8DF2EE23EBA291B2A9879DF4A5494D30DD` |

Total size is approximately 12.4 MB. Crash dumps themselves remain excluded
because they can contain machine memory and user data.

