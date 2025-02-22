//
// C++ Implementation: karaokelyricstextlrc
//
// Description:
//
//
// Author: Team XBMC <>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <math.h>

#include "Util.h"
#include "FileSystem/File.h"
#include "AdvancedSettings.h"
#include "MathUtils.h"
#include "utils/log.h"

#include "karaokelyricstextlrc.h"

CKaraokeLyricsTextLRC::CKaraokeLyricsTextLRC( const CStdString & lyricsFile )
  : CKaraokeLyricsText()
{
  m_lyricsFile = lyricsFile;
}


CKaraokeLyricsTextLRC::~CKaraokeLyricsTextLRC()
{
}

bool CKaraokeLyricsTextLRC::Load()
{
  enum ParserState
  {
    PARSER_INIT,    // looking for time
    PARSER_IN_TIME,    // processing time
    PARSER_IN_LYRICS  // processing lyrics
  };

  XFILE::CFile file;

  // Clear the lyrics array
  clearLyrics();

  if ( !file.Open( m_lyricsFile ) )
    return false;

  unsigned int lyricSize = (unsigned int) file.GetLength();

  if ( !lyricSize )
  {
    CLog::Log( LOGERROR, "LRC lyric loader: lyric file %s has zero length", m_lyricsFile.c_str() );
    return false;
  }

  // Read the file into memory array
  std::vector<char> lyricData( lyricSize );

  file.Seek( 0, SEEK_SET );

  // Read the whole file
  if ( file.Read( &lyricData[0], lyricSize) != lyricSize )
    return false; // disk error?

  file.Close();

  // Parse the correction value
  int timing_correction = MathUtils::round_int( g_advancedSettings.m_karaokeSyncDelayLRC * 10 );

  //
  // A simple state machine to parse the file
  //
  ParserState state = PARSER_INIT;
  unsigned int state_offset = 0;
  unsigned int lyric_flags = 0;
  int lyric_time = -1;
  int start_offset = 0;
  unsigned int offset = 0;

  CStdString ext, songfilename = getSongFile();
  CUtil::GetExtension( songfilename, ext );

  // Skip windoze UTF8 file prefix, if any, and reject UTF16 files
  if ( lyricSize > 3 )
  {
    if ( (unsigned char)lyricData[0] == 0xFF && (unsigned char)lyricData[1] == 0xFE )
    {
      CLog::Log( LOGERROR, "LRC lyric loader: lyrics file is in UTF16 encoding, must be in UTF8" );
      return false;
    }

    // UTF8 prefix added by some windoze apps
    if ( (unsigned char)lyricData[0] == 0xEF && (unsigned char)lyricData[1] == 0xBB && (unsigned char)lyricData[2] == 0xBF )
      offset = 3;
  }

  for ( char * p = &lyricData[offset]; offset < lyricSize; offset++, p++ )
  {
    // Skip \r
    if ( *p == 0x0D )
      continue;

    if ( state == PARSER_IN_LYRICS )
    {
      // Lyrics are terminated either by \n or by [
      if ( *p == '\n' || *p == '[' )
      {
        // Time must be there
        if ( lyric_time == -1 )
        {
          CLog::Log( LOGERROR, "LRC lyric loader: lyrics file has no time before lyrics" );
          return false;
        }

        // Add existing lyrics
        char current = *p;
        CStdString text;

        if ( offset > state_offset )
        {
          // null-terminate string, we saved current char anyway
          *p = '\0';
          text = &lyricData[0] + state_offset;
        }
        else
          text = " "; // add a single space for empty lyric

        // If this was end of line, set the flags accordingly
        if ( current == '\n' )
        {
          // Add space after the trailing lyric in lrc
          text += " ";
          addLyrics( text, lyric_time, lyric_flags | LYRICS_CONVERT_UTF8 );
          state_offset = -1;
          lyric_flags = CKaraokeLyricsText::LYRICS_NEW_LINE;
          state = PARSER_INIT;
        }
        else
        {
          // No conversion needed as the file should be in UTF8 already
          addLyrics( text, lyric_time, lyric_flags | LYRICS_CONVERT_UTF8 );
          lyric_flags = 0;
          state_offset = offset + 1;
          state = PARSER_IN_TIME;
        }

        lyric_time = -1;
      }
    }
    else if ( state == PARSER_IN_TIME )
    {
      // Time is terminated by ] or >
      if ( *p == ']' || *p == '>' )
      {
        int mins, secs, htenths, ltenths = 0;

        if ( offset == state_offset )
        {
          CLog::Log( LOGERROR, "LRC lyric loader: empty time" );
          return false; // [] - empty time
        }

        // null-terminate string
        char * timestr = &lyricData[0] + state_offset;
        *p = '\0';

        // Now check if this is time field or info tag. Info tags are like [ar:Pink Floyd]
        char * fieldptr = strchr( timestr, ':' );
        if ( timestr[0] >= 'a' && timestr[0] <= 'z' && timestr[1] >= 'a' && timestr[1] <= 'z' && fieldptr )
        {
          // Null-terminate the field name and switch to the field value
          *fieldptr = '\0';
          fieldptr++;

          while ( isspace( *fieldptr ) )
            fieldptr++;

          // Check the info field
          if ( !strcmp( timestr, "ar" ) )
            m_artist += fieldptr;
          else if ( !strcmp( timestr, "sr" ) )
          {
            // m_artist += "[CR]" + CStdString( fieldptr ); // Add source to the artist name as a separate line
          }
          else if ( !strcmp( timestr, "ti" ) )
            m_songName = fieldptr;
          else if ( !strcmp( timestr, "offset" ) )
          {
            if ( sscanf( fieldptr, "%d", &start_offset ) != 1 )
            {
              CLog::Log( LOGERROR, "LRC lyric loader: invalid [offset:] value '%s'", fieldptr );
              return false; // [] - empty time
            }

            // Offset is in milliseconds; convert to 1/10 seconds
            start_offset /= 100;
          }

          state_offset = -1;
          state = PARSER_INIT;
          continue;
        }
        else if ( sscanf( timestr, "%d:%d.%1d%1d", &mins, &secs, &htenths, &ltenths ) == 4 )
          lyric_time = mins * 600 + secs * 10 + htenths + MathUtils::round_int( ltenths / 10 );
        else if ( sscanf( timestr, "%d:%d.%1d", &mins, &secs, &htenths ) == 3 )
          lyric_time = mins * 600 + secs * 10 + htenths;
        else if ( sscanf( timestr, "%d:%d", &mins, &secs ) == 2 )
          lyric_time = mins * 600 + secs * 10;
        else
        {
          // bad time
          CLog::Log( LOGERROR, "LRC lyric loader: lyrics file has no proper time field: '%s'", timestr );
          return false;
        }

        // Correct timing if necessary
        lyric_time += start_offset;
        lyric_time += timing_correction;

        if ( lyric_time < 0 )
          lyric_time = 0;

        // Set to next char
        state_offset = offset + 1;
        state = PARSER_IN_LYRICS;
      }
    }
    else if ( state == PARSER_INIT )
    {
      // Ignore spaces
      if ( *p == ' ' || *p == '\t' )
        continue;

      // We're looking for [ or <
      if ( *p == '[' || *p == '<' )
      {
        // Set to next char
        state_offset = offset + 1;
        state = PARSER_IN_TIME;

        lyric_time = -1;
      }
      else if ( *p == '\n' )
      {
        // If we get a newline and we're not paragraph, set it
        if ( lyric_flags & CKaraokeLyricsText::LYRICS_NEW_LINE )
          lyric_flags = CKaraokeLyricsText::LYRICS_NEW_PARAGRAPH;
      }
      else
      {
        // Everything else is error
        CLog::Log( LOGERROR, "LRC lyric loader: lyrics file does not start from time" );
        return false;
      }
    }
  }

  return true;
}
