
/* Copyright (c) 2010, Peter Barrett  
**  
** Permission to use, copy, modify, and/or distribute this software for  
** any purpose with or without fee is hereby granted, provided that the  
** above copyright notice and this permission notice appear in all copies.  
**  
** THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL  
** WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED  
** WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR  
** BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES  
** OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,  
** WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,  
** ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS  
** SOFTWARE.  
*/

//#include <stdlib.h>
#include "Platform.h"
#include "Scroller.h"

#define _SHAKE_LIMIT 9
#define _PICS_W 320

class ViewState
{
	enum FileType
	{
		UNKNOWN,
		IM2,
		BLB,
		PNG,
		MP3,
		JPG,
		TXT
	};
	char		_name[16];
	FileType	_type;
	File		_file;
	Scroller	_scroller;
	int			_height;
    int         _sillycounter;
    int         _pics_count;
    int         _current_pic;
    char        _buffer [10];
    signed char _xyz[3];
    signed char _last_reading;

    void Draw(long scroll, int y, int height)
	{
		switch (_type)
		{
			case IM2:
				_file.SetPos(0);
				Graphics.DrawImage(_file,0,y,scroll,height);
				break;

			default:	
				Graphics.Rectangle(0,y,240,height,0xFFFF);
				Graphics.DrawString(_name,10,10,0);
				//TRACE("s: %ld, y:%ld\n",scroll,y);
				break;
		}
	}

	static void OnDraw(long scroll, int y, int height, void* ref)
	{
		((ViewState*)ref)->Draw(scroll,y,height);
	}

	void OpenIM2()
	{
		Img2 hdr;    
		if (!_file.Read(&hdr,sizeof(Img2)) || hdr.sig[0] != 'i' || hdr.sig[3] != '2')
			return;
		_height = hdr.height;
	}

	void DrawUnknown()
	{
		Graphics.DrawString(_name,10,10,0);
		_height = 320;
	}

public:
	int OnEvent(Event* e)
	{
		// _scroller.OnEvent(e);
		switch (e->Type)
		{
			case Event::OpenApp:
				{
					Graphics.Clear(0xFFFF);
					const char* name = "INIT.IM2";

					_file.Init();
					_file.Open(name);
					strcpy(_name,name);
					_height = 320;
                    _current_pic = 0;
                    Hardware.GetAccelerometer(_xyz);
                    _last_reading = _xyz[2];

					const char* ext = name;
					while (*ext && *ext != '.')
						ext++;

					//	Determine file type
					_type = UNKNOWN;
					if (ext[0] == '.')
					{
						if (ext[1] == 'I' && ext[2] == 'M' && ext[3] == '2')
							_type = IM2;
						else if (ext[1] == 'B' && ext[2] == 'L' && ext[3] == 'B')
							_type = BLB;
						//else if (ext[1] == 'J' && ext[2] == 'P' && ext[3] == 'G')
						//	_type = JPG;
					}

					switch (_type)
					{
						case IM2:
							OpenIM2();
							break;
						default:;
					}

					// Init scroller, do initial draw
					_scroller.Init(_height,OnDraw,this,320);
                    _pics_count = (_height / _PICS_W);
				}
				break;

			case Event::TouchDown:
				{
					TouchData* t = e->Touch;
					if (t->y >= 320)
						return -1;	// touched black bar quit
				}
				break;

			case Event::TouchMove:
				break;

			case Event::TouchUp:
				break;

			case Event::None:
                Hardware.GetAccelerometer(_xyz);
                
                if ((_last_reading - _xyz[2] > _SHAKE_LIMIT) || (_xyz[2] - _last_reading > _SHAKE_LIMIT)) {
                  // Tiem to scroll
                  _current_pic += 1;
                  if (_current_pic < _pics_count) {
                    _scroller.ScrollBy(-_PICS_W);
                  } else {
                    _current_pic = 0;
                    _scroller.ScrollBy(_height - _PICS_W);
                  }
                  
                  delay(500);

                  Hardware.GetAccelerometer(_xyz);
                  _last_reading = _xyz[2];
                  _sillycounter = 0;

                } else {
                  _last_reading = _xyz[2];

                  _sillycounter = (_sillycounter + 1) % 5000;
                  if (!_sillycounter) {
                    // Tiem to scroll
                    _current_pic += 1;
                    if (_current_pic < _pics_count) {
                      _scroller.ScrollBy(-_PICS_W);
                    } else {
                      _current_pic = 0;
                      _scroller.ScrollBy(_height - _PICS_W);
                    }
                  }
                
                //Graphics.Rectangle(0,0,240,10,0xFFFF);
                //itoa (_sillycounter,_buffer,10);
                //Graphics.DrawString(_buffer,0,0,0);
                }
				break;

			default:;
		}
		return 0;
	}
};

INSTALL_APP(shell,ViewState);