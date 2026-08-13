# Repository Guidelines

## Project Structure & Module Organization

This repository is currently empty aside from Git metadata, so keep the initial layout simple and predictable as code is added. Place application source under `src/`, automated tests under `tests/`, and static or generated assets under `assets/` when needed. Use `docs/` for contributor-facing documentation beyond this file.

Prefer small, purpose-named modules over broad utility files. For example, group feature code as `src/<feature>/` and mirror that structure in `tests/<feature>/` so behavior is easy to locate.

## Build, Test, and Development Commands

No build or test tooling is configured yet. When tooling is introduced, document the canonical commands here and keep them stable for contributors.

Expected examples:

```sh
npm install      # install JavaScript dependencies, if this becomes a Node project
npm run dev      # start the local development server
npm test         # run the test suite
```

If the project uses another stack, replace these examples with the actual commands from its package manager or build tool, such as `cargo test`, `pytest`, `make build`, or `cmake --build`.

## Coding Style & Naming Conventions

Follow the formatter and linter configured for the chosen language. Until those tools exist, keep formatting consistent within each file, use descriptive names, and avoid unrelated refactors in feature changes.

Use lower-case, hyphenated names for documentation files when practical, such as `docs/setup-guide.md`. For code, follow the language default: `camelCase` for JavaScript variables, `PascalCase` for classes/components, `snake_case` for Python functions, and clear test names that describe behavior.

## Testing Guidelines

Add tests with new behavior whenever practical. Keep unit tests close to the code they verify through mirrored paths, for example `tests/parser/parser.test.js` for `src/parser/`.

Name tests by expected behavior rather than implementation details. Before opening a pull request, run the full test command once it exists and include any relevant manual verification notes.

## Commit & Pull Request Guidelines

There is no commit history yet, so no repository-specific convention has been established. Use concise, imperative commit subjects such as `Add initial project structure` or `Fix config loading`.

Pull requests should include a short summary, testing performed, linked issues when applicable, and screenshots or recordings for user-facing UI changes. Keep PRs focused on one logical change so review remains straightforward.
