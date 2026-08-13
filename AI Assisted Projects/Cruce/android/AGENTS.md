# Repository Guidelines

## Project Structure & Module Organization

This is an Android Gradle project named `CruceAndroid` with a single application module, `:app`. Keep app code under `app/src/main/`; the current package root is `app/src/main/java/com/cruce/android/`. Android resources belong in `app/src/main/res/`, and manifest changes belong in `app/src/main/AndroidManifest.xml`.

Use standard Android test locations as the project grows: unit tests in `app/src/test/` and device or emulator tests in `app/src/androidTest/`. Avoid committing generated output from `build/`, `.gradle/`, or IDE cache directories.

## Build, Test, and Development Commands

Run commands from the repository root using the Gradle wrapper:

```sh
.\gradlew.bat assembleDebug
.\gradlew.bat installDebug
.\gradlew.bat testDebugUnitTest
.\gradlew.bat connectedDebugAndroidTest
.\gradlew.bat clean
```

`assembleDebug` builds a debug APK. `installDebug` installs it on a connected device or emulator. `testDebugUnitTest` runs local JVM unit tests once they exist. `connectedDebugAndroidTest` runs instrumented Android tests and requires a running device or emulator. `clean` removes Gradle build outputs.

## Coding Style & Naming Conventions

Use Kotlin and Java 17 conventions already configured in Gradle. Indent Kotlin with four spaces, keep imports organized, and prefer small, focused classes over broad utility files. Use `PascalCase` for activities, classes, and composables; `camelCase` for functions, properties, and local variables; and lower-case resource names with underscores, such as `activity_main.xml` or `primary_button_bg.xml`.

Keep package names under `com.cruce.android` unless there is a deliberate module or product rename.

## Testing Guidelines

Add tests with new behavior when practical. Mirror production package structure under `app/src/test/java/` for unit tests and `app/src/androidTest/java/` for instrumented tests. Name tests by expected behavior, for example `loadsDefaultState()` or `rendersErrorWhenConnectionFails()`.

Before a pull request, run `.\gradlew.bat testDebugUnitTest`. Run `.\gradlew.bat connectedDebugAndroidTest` for changes involving Android framework behavior, lifecycle handling, storage, permissions, or UI.

## Commit & Pull Request Guidelines

This repository has no commit history yet, so no project-specific convention is established. Use concise, imperative commit subjects such as `Add Android app module` or `Fix startup state handling`.

Pull requests should include a short summary, tests run, linked issues when applicable, and screenshots or recordings for visible UI changes. Keep each PR focused on one logical change.

## Security & Configuration Tips

Do not commit local SDK paths, credentials, signing keys, or machine-specific settings. Keep `local.properties` local, and use Gradle properties or documented environment variables for configuration that contributors need to reproduce.
