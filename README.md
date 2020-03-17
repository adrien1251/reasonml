# back


[![CircleCI](https://circleci.com/gh/yourgithubhandle/back/tree/master.svg?style=svg)](https://circleci.com/gh/yourgithubhandle/back/tree/master)


**Contains the following libraries and executables:**

```
back@0.0.0
│
├─test/
│   name:    TestBack.exe
│   main:    TestBack
│   require: back.lib
│
├─library/
│   library name: back.lib
│   namespace:    Back
│   require:
│
└─executable/
    name:    BackApp.exe
    main:    BackApp
    require: back.lib
```

## Developing:

```
npm install -g esy
git clone <this-repo>
esy install
esy build
```

## Running Binary:

After building the project, you can run the main binary that is produced.

```
esy x BackApp.exe 
```

## Running Tests:

```
# Runs the "test" command in `package.json`.
esy test
```
