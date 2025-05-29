# cite

A command-line tool for generating formatted bibliographies and footnotes from JSON files based on the BibJSON schema. Supports Chicago-style output with proper bibliography ordering and both long and short footnotes. Outputs can be generated as Markdown or HTML.

## Features

- Parses JSON files based on BibJSON conventions (see [SCHEMA.md](SCHEMA.md) for details)
- Outputs:
  - Bibliography (ordered by author last name for Chicago)
  - Long and short Chicago-style footnotes
- Output formats:
  - Markdown (default, prints to terminal or writes to `.md`)
  - HTML (when output filename ends with `.html`)
- Handles multi-author and complex fields

## Usage

```sh
./cite export input.json chicago output.md
```

- `input.json` — your JSON file (see [SCHEMA.md](SCHEMA.md))
- `chicago` — citation style (currently only Chicago is implemented)
- `output.md` or `output.html` — output file (optional)
  - If omitted, outputs Markdown to terminal

Examples:

- Markdown to terminal:
  ```
  ./cite export mybib.json chicago
  ```
- Markdown to file:
  ```
  ./cite export mybib.json chicago bibliography.md
  ```
- HTML to file:
  ```
  ./cite export mybib.json chicago bibliography.html
  ```

## Input Format

Your input file should be valid JSON, either as an array of records or as a collection object with a `"records"` key.  
See [SCHEMA.md](SCHEMA.md) for details and a full example.

## Output

- **Chicago**:  
  - Alphabetically ordered bibliography  
  - Long footnotes and short footnotes  
  - Footnotes include `[pg]` as a placeholder for page number

## TODO

- [ ] Add an interactive tool to add new entries to your JSON file (or create one). Should prompt for type of source, then relevant fields (title, authors, etc.) and output valid JSON.
- [ ] Add MLA format
- [ ] Add IEEE format
- [ ] Add APA format

---

MIT License. See [LICENSE](LICENSE).
