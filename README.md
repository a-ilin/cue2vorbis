cue2vorbis
==========

Print CUE file tags in Vorbis naming.

This program outputs CUE file tags in Vorbis format, as required for FLAC and Ogg audio files.


## Usage

Usage: `cue2vorbis <CUE file> [track number]`

* When the track number is given, the tool will print tags only for the specified track.
* When no track number is given, the tool will print tags for all tracks from the CUE file. Each track entry is started with the tag TRACKNUMBER (may be useful in scripting).

Example command line on providing tags to `metaflac` program for 4th track:
```sh
cue2vorbis album.cue 4 | metaflac --import-cuesheet-from="-" track_4.flac
```

## Compilation

Install dependencies, compile and install:
```sh
sudo apt install build-essential libcue libcue-dev

make

sudo make install
```

## Dependencies

* (libcue)[https://github.com/lipnitsk/libcue]


## License

**GPLv2**

