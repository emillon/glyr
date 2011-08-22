/***********************************************************
* This file is part of glyr
* + a commnadline tool and library to download various sort of musicrelated metadata.
* + Copyright (C) [2011]  [Christopher Pahl]
* + Hosted at: https://github.com/sahib/glyr
*
* glyr is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* glyr is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with glyr. If not, see <http://www.gnu.org/licenses/>.
**************************************************************/
#include "../../stringlib.h"
#include "../../core.h"

const char * tracklist_musicbrainz_url(GlyrQuery * sets)
{
    return "http://musicbrainz.org/ws/1/release/?type=xml&artist=%artist%&releasetypes=Official&limit=10&title=%album%&limit=1";
}

#define REL_ID_BEGIN "id=\""
#define REL_ID_ENDIN "\" ext:score="
#define REL_ID_FORM  "http://musicbrainz.org/ws/1/release/%s?type=xml&inc=tracks"

#define DUR_BEGIN "<duration>"
#define DUR_ENDIN "</duration>"
#define TIT_BEGIN "<title>"
#define TIT_ENDIN "</title>"

/* ----------------------------------------- */

static GList * traverse_xml(const gchar * data, const gchar * url, cb_object * capo)
{
    gchar * beg = (gchar*)data;
    GList * collection = NULL;
    gint ctr = 0;

    while(continue_search(ctr,capo->s) && (beg = strstr(beg+1,TIT_BEGIN)) != NULL)
    {
        gchar * dy;
        gchar * value = copy_value(beg+strlen(TIT_BEGIN),strstr(beg,TIT_ENDIN));
        gchar * durat = copy_value(strstr(beg,DUR_BEGIN)+strlen(DUR_BEGIN),(dy = strstr(beg,DUR_ENDIN)));
        if(value != NULL && durat != NULL)
        {
            GlyrMemCache * cont = DL_init();
            cont->data = beautify_lyrics(value);
            cont->size = strlen(cont->data);
            cont->duration = atoi(durat) / 1e3;
            cont->dsrc = strdup(url);
            collection = g_list_prepend(collection,cont);

            ctr++;

            /* free & jump to next */
            g_free(value);
            g_free(durat);
            beg = dy;
        }
    }
    return collection;
}

/* ----------------------------------------- */

/* Use simple text parsing, xml parsing has no advantage here */
GList * tracklist_musicbrainz_parse(cb_object * capo)
{
    GList * ls = NULL;
    gchar * release_ID = NULL;

    if( (release_ID = copy_value(strstr(capo->cache->data,REL_ID_BEGIN)+strlen(REL_ID_BEGIN),strstr(capo->cache->data,REL_ID_ENDIN))) != NULL)
    {
        char * release_page_info_url = g_strdup_printf(REL_ID_FORM,release_ID);
        GlyrMemCache * dlData = download_single(release_page_info_url,capo->s,NULL);
        if(dlData)
        {
            ls = traverse_xml(dlData->data,capo->url,capo);
            DL_free(dlData);
        }
        g_free(release_page_info_url);
        g_free(release_ID);
    }
    return ls;
}

/*--------------------------------------------------------*/

MetaDataSource tracklist_musicbrainz_src =
{
    .name = "musicbrainz",
    .key  = 'm',
    .parser    = tracklist_musicbrainz_parse,
    .get_url   = tracklist_musicbrainz_url,
    .quality   = 90,
    .speed     = 90,
    .endmarker = NULL,
    .free_url  = false,
    .type      = GET_TRACKLIST
};
