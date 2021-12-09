#pragma once

#include <ESP32-Chimera-Core.h>
extern LGFX &tft;

#include <map>

enum imgType_t
{
  IMGFONT_PNG,
  IMGFONT_BMP,
  IMGFONT_RAW
};

// monospace font sprite
class imgFont_t
{
  public:
    imgFont_t(
      const char* _name,
      imgType_t _type,
      uint8_t _bit_depth,
      const unsigned char* _data,
      size_t _data_len,
      const bool _force_upppercase,
      const char* _chars,
      const size_t _chars_len,
      uint32_t _width,
      uint32_t _height,
      uint32_t _fontWidth,
      uint32_t _fontHeight,
      uint32_t _transcolor
    ) :
      name(_name),
      type(_type),
      bit_depth(_bit_depth),
      data(_data),
      data_len(_data_len),
      force_upppercase(_force_upppercase),
      chars(_chars),
      chars_len(_chars_len),
      width(_width),
      height(_height),
      fontWidth(_fontWidth),
      fontHeight(_fontHeight),
      transcolor(_transcolor)
    {
      initchars();
    }

    const char* name = nullptr;

    imgType_t type; // png/bmp/raw @ 1bpp
    uint8_t bit_depth;

    const unsigned char* data;
    size_t data_len;

    const char* chars;  // ordered characters from the sprite image
    const size_t chars_len;   // characters count in sprite
    bool force_upppercase; // if the character set is limited

    int32_t width;     // sprite width
    int32_t height;    // sprite height

    const int32_t fontWidth; // char width
    const int32_t fontHeight;// char height

    uint32_t transcolor;        // invert 1bit colors

    void draw( LGFX_Sprite* fontSprite, int32_t charindex, float scale )
    {
      //uint32_t dest_x = (charindex*fontWidth);
      //sptr->pushRotateZoom( fontSprite, (-dest_x*scale + scale*width/2)+1, (scale*height/2)+1, 0, scale, scale );
      //, bit_depth, fontSprite->getPalette()
      //if( bit_depth == 1 ) {
        fontSprite->pushImageRotateZoom( 1, 1, charindex*fontWidth, 0, 0, scale, scale, width, height, data, transcolor, (lgfx::v1::color_depth_t)bit_depth, fontSprite->getPalette() );
      //} else {
        //fontSprite->pushImageRotateZoomWithAA( 1, 1, charindex*fontWidth, 0, 0, scale, scale, width, height, data, transcolor, (lgfx::v1::color_depth_t)bit_depth, sptr->getPalette() );
      //}
    }

    void toRaw()
    {
      if( sptr == nullptr ) {
        sptr = new LGFX_Sprite( &tft );
        sptr->setColorDepth( bit_depth );
        int vdiff = fontHeight-height;
        uint32_t needed_size = chars_len*fontWidth;

        if( width != needed_size ) {
          // uncondensed, glue all chars without the separator line
          bool sizewithtrail = width == needed_size+chars_len+1; // pixel separated map with trailing row
          bool sizenotrail   = width == needed_size+chars_len;   // pixel separated map without trailing row
          if( sizewithtrail || sizenotrail ) {
            width = needed_size;
            uint32_t heapnow = ESP.getFreeHeap();
            if(! sptr->createSprite( width, fontHeight ) ) {
              log_e("Failed to create sprite, aborting");
              return;
            }
            log_w("This font consumes %d bytes", heapnow-ESP.getFreeHeap() );
            for( int i=0; i<chars_len; i++ ) {
              int32_t offset_x = ( (fontWidth+1) * i)+1;
              int32_t offset_y = 0;//vdiff!=0 ? 1 : 0;
              int32_t dest_x   = (i*fontWidth);
              int32_t dest_y   = -1;
              sptr->drawPng( data, data_len, dest_x, dest_y, fontWidth, fontHeight+1, offset_x, offset_y );
            }
          }
        }
        type = IMGFONT_RAW;
        height = fontHeight;
        data = (const unsigned char*)sptr->getBuffer();
      }
    }
    int16_t getCharIndex( unsigned char c ) {
      if( force_upppercase ) c = toupper(c);
      return chars_indexes[c];
    }
  private:
    LGFX_Sprite *sptr;   // raw data holder for png/bmp formats
    int16_t chars_indexes[255];
    char startcode=255, endcode=0;

    void initchars()
    {
      // fill with -1
      uint8_t minc = 255, maxc = 0;
      for( int i=0; i<255;i++ ) {
        chars_indexes[i] = -1;
      }
      for( int i=0; i<chars_len;i++ ) {
        chars_indexes[chars[i]] = i;
        startcode = min( startcode, chars[i] );
        endcode = max( endcode, chars[i] );
      }
      log_d("Startcode=%d, endcode=%d", startcode, endcode );
    }
};



void drawImgChar( LGFX_Sprite* fontSprite, char ch, int32_t x, int32_t y, float scale, uint32_t color, int64_t _bgcolor=-1, imgFont_t *iFont=nullptr )
{
  uint32_t bgcolor = _bgcolor;

  if( bgcolor < 0 ) {
    // no background, elect transparent color for 1bit palette
    bgcolor = (color!=0) ? 0x000000U : 0xffffffU-color;
  }

  float outWidth  = scale*iFont->fontWidth;
  float outHeight = scale*iFont->fontHeight;
  //uint32_t transcolorIndex = iFont->type==IMGFONT_RAW?1:0;
  int charIdx = iFont->getCharIndex( ch );
  if( charIdx < 0 ) return;

  if( iFont->type != IMGFONT_RAW ) {
    iFont->toRaw();
  }

  fontSprite->setColorDepth( iFont->bit_depth );
  fontSprite->createSprite( outWidth, outHeight );
  //fontSprite->fillSprite( bgcolor );
  if( iFont->bit_depth == 1) {
    fontSprite->setPaletteColor( 0, iFont->transcolor==1?color:bgcolor );
    fontSprite->setPaletteColor( 1, iFont->transcolor==1?bgcolor:color );
  }
  //fontSprite->pushImageRotateZoom( 0, 0, charIdx*iFont->fontWidth, 0, 0, scale, scale, iFont->width, iFont->height, iFont->data, lgfx::palette_1bit, fontSprite->getPalette() );

  iFont->draw( fontSprite, charIdx, scale );

  if( _bgcolor<0 ) { // use native transparency
    fontSprite->pushSprite( x, y, iFont->transcolor );
  } else if( iFont->bit_depth>1 && _bgcolor!=iFont->transcolor ) { // bgcolor provided, fill first
    tft.fillRect( x, y, iFont->fontWidth*scale, iFont->fontHeight*scale, (uint32_t)_bgcolor );
    fontSprite->pushSprite( x, y, iFont->transcolor );
  } else { //
    fontSprite->pushSprite( x, y );
  }
  fontSprite->deleteSprite();

}


struct charDrawer_t
{
  uint32_t now;
  uint32_t posy = 10;
  float scale = 4.0;
  bool drawlines = false;

  void drawString( LGFX_Sprite* fontSprite, String text, int32_t x, int32_t y, imgFont_t* font, int64_t color=0x00bb00U, int64_t bgcolor=-1, float scale=4.0  )
  {
    int hbefore = font->height;
    int wbefore = font->width;
    //drawImgChar( fontSprite, 'M', -100, -100, 3.0, color, 0xffffff, font ); // init char sprite
    //now = millis();
    int lineheight = font->fontHeight*scale;
    int charwidth  = font->fontWidth*scale + scale;
    if( drawlines ) tft.drawFastHLine( 0, posy,            tft.width(), 0xff8080U );
    if( drawlines ) tft.drawFastHLine( 0, posy+lineheight, tft.width(), 0x8080ffU );
    //String text = apptitle;
    //text.toUpperCase();
    for( int i=0;i<text.length();i++ ) {
      uint32_t posx = x+(i*charwidth);
      if( drawlines ) tft.drawFastVLine( posx, posy, lineheight, 0x80ff80U );
      drawImgChar( fontSprite, text[i], posx, y, scale, (uint32_t)color, bgcolor, font );
    }
    //Serial.printf("%s: %d ms @%dbpp sprite[(%3d => %-3d) x (%2d => %-2d)] char[%2d x %-2d] \n", font->name, millis()-now, font->bit_depth, wbefore, font->width, hbefore, font->height, font->fontWidth, font->fontHeight );
    //now = millis();
  }
};
