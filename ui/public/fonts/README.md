# Self-hosted fonts

All font files are vendored locally so the WebView renders with zero CDN
dependency (it may be offline). Each family is declared via `@font-face` in
`src/styles/globals.css`.

| Role | Family | Files | License |
| --- | --- | --- | --- |
| Display | Akira Expanded (Super Bold, 800–900) | `AkiraExpanded-SuperBold.woff2` | Foundry demo font — free for evaluation/personal use; obtain a commercial license for distribution |
| UI mono | JetBrains Mono (Regular 400, Bold 700) | `JetBrainsMono-Regular.woff2`, `JetBrainsMono-Bold.woff2` | [SIL Open Font License 1.1](https://openfontlicense.org) |
| ASCII / VGA | Px437 IBM VGA 8x16 | `Px437_IBM_VGA_8x16.ttf` | [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) — by VileR, [The Ultimate Oldschool PC Font Pack](https://int10h.org/oldschool-pc-fonts/) |

## Sources

- Akira Expanded: demo WOFF2 pulled from a public font mirror
  (`009fly/akirafontforwebsite`). Contains the full uppercase alphabet plus
  numerals and basic punctuation — sufficient for the SYNTHORTION wordmark and
  section codes.
- JetBrains Mono: `JetBrains/JetBrainsMono` GitHub repository webfonts.
- Px437 IBM VGA 8x16: `potatoes1286/oldschool-pc-fonts-gh` mirror of VileR's
  font pack (pixel-outline TTF). Includes box-drawing `┌┐└┘` and block elements.
