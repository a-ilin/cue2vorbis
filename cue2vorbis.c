/*
 * Copyright (c) 2024, Aleksei Ilin
 *
 * For license terms, see the file COPYING in this distribution.
 */

#include <libcue.h>
#include <stdio.h>
#include <stdlib.h>

// Vorbis tag references:
// * https://wiki.hydrogenaud.io/index.php?title=Tag_Mapping
// * https://age.hobba.nl/audio/mirroredpages/ogg-tagging.html

typedef struct PtiNamed {
    // libcue identifier.
    Pti pti;
    // Tag name to print.
    const char* name;
} PtiNamed;

typedef struct RemNamed {
    // libcue identifier.
    RemType rem;
    // Tag name to print.
    const char* name;
} RemNamed;

// Common tags for track and album.
const static PtiNamed g_pti_array[] = {
    { PTI_SONGWRITER,  "LYRICIST",  },
    { PTI_COMPOSER,    "COMPOSER",  },
    { PTI_ARRANGER,    "ARRANGER",  },
    { PTI_MESSAGE,     "COMMENT",   },
    { PTI_GENRE,       "GENRE",     },
};

// Common rems for track and album.
const static RemNamed g_rem_array[] = {
    { REM_DATE,                  "DATE",                  },
    { REM_REPLAYGAIN_ALBUM_GAIN, "REPLAYGAIN_ALBUM_GAIN", },
    { REM_REPLAYGAIN_ALBUM_PEAK, "REPLAYGAIN_ALBUM_PEAK", },
    { REM_REPLAYGAIN_TRACK_GAIN, "REPLAYGAIN_TRACK_GAIN", },
    { REM_REPLAYGAIN_TRACK_PEAK, "REPLAYGAIN_TRACK_PEAK", },
};

int print_track(int track_number, int track_count, Cd* cue_cd)
{
    int ret_code = 0;
    Track* cue_track = NULL;
    Cdtext* cue_cd_cdtext = NULL;
    Rem* cue_cd_rem = NULL;
    Cdtext* cue_track_cdtext = NULL;
    Rem* cue_track_rem = NULL;
    const char* text = NULL;

    cue_track = cd_get_track(cue_cd, track_number);
    if (!cue_track) {
        fprintf(stderr, "Cannot get track from CD.\n");
        goto onerror;
    }

    // Get CD CDTEXT.
    cue_cd_cdtext = cd_get_cdtext(cue_cd);
    if (!cue_cd_cdtext) {
        fprintf(stderr, "Cannot get CDTEXT metadata from CD.\n");
        goto onerror;
    }

    // Get CD REM.
    cue_cd_rem = cd_get_rem(cue_cd);
    if (!cue_cd_rem) {
        fprintf(stderr, "Cannot get REM metadata from CD.\n");
        goto onerror;
    }

    // Get track CDTEXT.
    cue_track_cdtext = track_get_cdtext(cue_track);
    if (!cue_track_cdtext) {
        fprintf(stderr, "Cannot get CDTEXT metadata from track.\n");
        goto onerror;
    }

    // Get track REM.
    cue_track_rem = track_get_rem(cue_track);
    if (!cue_track_rem) {
        fprintf(stderr, "Cannot get REM metadata from track.\n");
        goto onerror;
    }

    // Print TRACK.
    printf("TRACKNUMBER=%d\n", track_number);
    printf("TRACKTOTAL=%d\n", track_count);

    // Print TITLE.
    text = cdtext_get(PTI_TITLE, cue_track_cdtext);
    if (text) {
        printf("TITLE=%s\n", text);
    }

    // Print ALBUM.
    text = cdtext_get(PTI_TITLE, cue_cd_cdtext);
    if (text) {
        printf("ALBUM=%s\n", text);
    }

    // Print ARTIST.
    text = cdtext_get(PTI_PERFORMER, cue_track_cdtext);
    if (text) {
        printf("ARTIST=%s\n", text);
    }

    // Print ALBUMARTIST.
    text = cdtext_get(PTI_PERFORMER, cue_cd_cdtext);
    if (text) {
        printf("ALBUMARTIST=%s\n", text);
    }

    // Print CDTEXT metadata.
    for (size_t i = 0; i < sizeof(g_pti_array) / sizeof(g_pti_array[0]); ++i) {
        text = cdtext_get(g_pti_array[i].pti, cue_track_cdtext);
        if (text) {
            printf("%s=%s\n", g_pti_array[i].name, text);
        } else {
            // No metadata for track, try CD.
            text = cdtext_get(g_pti_array[i].pti, cue_cd_cdtext);
            if (text) {
                printf("%s=%s\n", g_pti_array[i].name, text);
            }
        }
    }

    // Print REM metadata.
    for (size_t i = 0; i < sizeof(g_rem_array) / sizeof(g_rem_array[0]); ++i) {
        text = rem_get(g_rem_array[i].rem, cue_track_rem);
        if (text) {
            printf("%s=%s\n", g_rem_array[i].name, text);
        } else {
           // No metadata for track, try CD.
            text = rem_get(g_rem_array[i].rem, cue_cd_rem);
            if (text) {
                printf("%s=%s\n", g_rem_array[i].name, text);
            }
        }
    }

    // Print ISRC.
    text = cdtext_get(PTI_UPC_ISRC, cue_track_cdtext);
    if (!text) {
        text = track_get_isrc(cue_track);
    }
    if (text) {
        printf("ISRC=%s\n", text);
    }

    // Print EAN/UPN.
    text = cdtext_get(PTI_UPC_ISRC, cue_cd_cdtext);
    if (text) {
        printf("EAN/UPN=%s\n", text);
    }

    goto cleanup;

onerror:
    ret_code = 1;

cleanup:
    return ret_code;
}

void usage(void)
{
    fprintf(stderr, "Print CUE file tags in Vorbis naming.\n");
    fprintf(stderr, "Usage: cue2vorbis <CUE file> [track number]\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "When the track number is given, the program will print tags only for the specified track.\n");
    fprintf(stderr, "When no track number is given, the program will print tags for all tracks from the CUE file.\n");
    fprintf(stderr, "Each track entry is started with the tag TRACKNUMBER. This may be used as a separator in scripting.\n");
    fprintf(stderr, "\n");
}

int main(int argc, char* argv[])
{
    int ret_code = 0;

    // Argument: CUE filename.
    const char* cue_fn = NULL;
    // Argument (optional): track number.
    int track_arg = 0;

    int track_count = 0;
    FILE* cue_file = NULL;
    Cd* cue_cd = NULL;

    if (argc < 2 || argc > 3) {
        usage();
        goto onerror;
    }

    cue_fn = argv[1];

    if (argc > 2) {
        track_arg = atoi(argv[2]);
        if (track_arg == 0) {
            fprintf(stderr, "Wrong track number: '%s'\n", argv[2]);
            goto onerror;
        }
    }

    cue_file = fopen(cue_fn, "r");
    if (!cue_file) {
        fprintf(stderr, "Cannot open CUE file: '%s'\n", cue_fn);
        goto onerror;
    }

    cue_cd = cue_parse_file(cue_file);
    if (!cue_cd) {
        fprintf(stderr, "Cannot parse CUE file.\n");
        goto onerror;
    }

    track_count = cd_get_ntrack(cue_cd);
    if (track_count == 0) {
        fprintf(stderr, "CUE file has no tracks.\n");
        goto onerror;
    }

    if (track_arg > track_count) {
        fprintf(stderr, "CUE file does not have track #%d.\n", track_arg);
        goto onerror;
    }

    if (track_arg != 0) {
        if (print_track(track_arg, track_count, cue_cd) != 0) {
            goto onerror;
        }
    } else {
        // Print all tracks.
        for (int i = 1; i <= track_count; ++i) {
            if (print_track(i, track_count, cue_cd) != 0) {
                goto onerror;
            }
        }
    }

    goto cleanup;

onerror:
    ret_code = 1;

cleanup:
    if (cue_cd) {
        cd_delete(cue_cd);
    }

    if (cue_file) {
        fclose(cue_file);
    }

    return ret_code;
}
