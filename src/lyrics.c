#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "core.h"
#include "types.h"
#include "stringop.h"

#include "lyrics/lyricswiki.h"
#include "lyrics/magistrix.h"
#include "lyrics/lyrix_at.h"
#include "lyrics/darklyrics.h"
#include "lyrics/lyricsvip.h"
#include "lyrics/directlyrics.h"
#include "lyrics/songlyrics.h"
#include "lyrics/lyrdb.h"
#include "lyrics/metrolyrics.h"

#include "books/dnb.h"
#include "books/kit.h"

// Add your's here
sk_pair_t lyric_providers[] =
{
//  full name       key  coloredname    use?    parser callback           geturl callback         free url?
    {"kit",         "k", "kit",         false, {books_kit_parse,          books_kit_url,          false}},
    {"dnb",         "n", "dnb",         false, {books_dnb_parse,          books_dnb_url,          false}},
    {"lyricswiki",  "w", "lyricswiki",  false, {lyrics_lyricswiki_parse,  lyrics_lyricswiki_url,  false}},
    {"darklyrics",  "y", "darklyrics",  false, {lyrics_darklyrics_parse,  lyrics_darklyrics_url,  false}},
    {"directlyrics","i", "directlyrics",false, {lyrics_directlyrics_parse,lyrics_directlyrics_url,true }},
    {"songlyrics",  "s", "songlyrics",  false, {lyrics_songlyrics_parse,  lyrics_songlyrics_url,  true }},
    {"lyricsvip",   "v", "lyricsvip",   false, {lyrics_lyricsvip_parse,   lyrics_lyricsvip_url,   true }},
    {"safe",        NULL,NULL,          false },
    {"lyr.db",      "d", "lyr.db",      false, {lyrics_lyrdb_parse,       lyrics_lyrdb_url,       false}},
    {"lyrix.at",    "a", "lyrix.at",    false, {lyrics_lyrixat_parse,     lyrics_lyrixat_url,     false}},
    {"metrolyrics", "m", "metrolyrics", false, {lyrics_metrolyrics_parse, lyrics_metrolyrics_url, false}},
    {"magistrix",   "x", "magistrix",   false, {lyrics_magistrix_parse,   lyrics_magistrix_url,   false}},
    {"unsafe",      NULL, NULL,         false },
    { NULL,         NULL, NULL,         false },
};

sk_pair_t * glyr_get_lyric_providers(void)
{
    return lyric_providers;
}

#define multi_fprintf(...) fprintf(handle, __VA_ARGS__); fprintf(stdout, __VA_ARGS__)

bool write_lyrics(const char * path, const char * lyrics, const char * artist, const char * album, const char * title)
{
    if(path)
    {
        FILE * handle = fopen(path,"w");
        if(handle)
        {
            printf("\n");
            //multi_fprintf("\"%s\" by \"%s\" ",title,artist);
            if(album)
            {
                //multi_fprintf("\nfrom album \"%s\"",album);
            }

            //multi_fprintf("\n-------------------------------------\n");
            multi_fprintf("%s\n",lyrics);
            //multi_fprintf("%s\n\n-------------------------------------\n",lyrics);
            //multi_fprintf("# This file was generated by glyr   #\n");
            //multi_fprintf("-------------------------------------\n");
            fclose(handle);

            return true;
        }
    }
    return false;
}

// let this be only a local phenomena
#undef multi_fprintf

const char * finalize(cb_object * plugin_list, size_t it, const char * dummy, glyr_settings_t * s)
{
    // Now do the complex multi download action in the background..
    memCache_t *result = invoke(plugin_list, it, LYRIC_MAX_PLUGIN,s->artist,s->album,s->title);

    if(result)
    {
        //printf("\n%s",beautify_lyrics(result->data));
        char * path = strdup_printf("%s/%s-%s.lyr",s->save_path,s->artist,s->title);
        char * blyr = beautify_lyrics(result->data);

        if(write_lyrics(path,blyr,s->artist,s->album,s->title) == false)
        {
            fprintf(stderr,C_R"Unable to write lyrics '"C_"%s"C_R"'\n"C_,path);
        }

        if(blyr)
            free(blyr);

        DL_free(result);
        return path;
    }

    return NULL;
}

const char * get_lyrics(glyr_settings_t * settings, const char * filename)
{
    if(settings == NULL || settings->artist == NULL || settings-> title == NULL)
    {
        fprintf(stderr,C_R"(!) "C_"<%s> is empty but needed for lyricsDL!\n",settings->artist ? "title":"artist");
        return NULL;
    }

    return register_and_execute(settings, filename, finalize);
}

