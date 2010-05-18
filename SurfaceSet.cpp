/*
 * Copyright 2010 Daniel Albano
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "SurfaceSet.h"

SurfaceSet::SurfaceSet(int width, int height) : _width(width), _height(height), _nframe(0), _frames()
{

}

/**
 * Deletes the image from memory.
 */
SurfaceSet::~SurfaceSet()
{
	delete _surface;
}

/**
 * Loads the contents of an X-Com set of PCK/TAB image files
 * into the surface. The PCK file contains an RLE compressed
 * image, while the TAB file contains the offsets to each
 * frame in the image.
 * @param filename Filename of the PCK image.
 * @sa http://www.ufopaedia.org/index.php?title=Image_Formats#PCK
 */
void SurfaceSet::loadPck(string filename)
{
	string pck = string();
	string tab = string();
	pck = filename.substr(0, filename.length()-4)+".PCK";
	tab = filename.substr(0, filename.length()-4)+".TAB";

	// Load TAB and get image offsets
	ifstream offsetFile (tab.c_str(), ios::in | ios::binary);
	if (!offsetFile)
	{
		_nframe = 1;
	}
	else
	{
		Uint16 off;

		while (offsetFile.read((char*)&off, sizeof(off)))
		{
			SDL_Rect rect;
			rect.x = 0;
			rect.y = _nframe*_height;
			rect.w = _width;
			rect.h = _height;
			_frames[_nframe] = rect;
			_nframe++;
		}
	}

	_surface = new Surface(_width, _height * _nframe);

	// Load PCX and put pixels in surface
    ifstream imgFile (pck.c_str(), ios::in | ios::binary);
	if (!imgFile)
	{
		throw "Failed to load PCK";
	}
	
	// Lock the surface
	SDL_LockSurface(_surface->getSurface());

	char value;
	
	for (int frame = 0; frame < _nframe; frame++)
	{
		int x = 0;
		int y = frame * _height;

		imgFile.read(&value, 1);
		for (int i = 0; i < value; i++)
		{
			for (int j = 0; j < _width; j++)
			{
				_surface->setPixelIterative(&x, &y, 0);
			}
		}
		
		while (imgFile.read(&value, 1))
		{
			if (value == -2)
			{
				imgFile.read(&value, 1);
				for (int i = 0; i < value; i++)
				{
					_surface->setPixelIterative(&x, &y, 0);
				}
			}
			else if (value == -1)
			{
				break;
			}
			else
			{
				_surface->setPixelIterative(&x, &y, Uint8(value));
			}
		}
	}
	
	/*
	if (!imgFile.eof())
	{
		throw "Invalid data from file";
	}
	*/

	// Unlock the surface
	SDL_UnlockSurface(_surface->getSurface());

	imgFile.close();
	offsetFile.close();
}

/**
 * Loads the contents of an X-Com DAT image file into the
 * surface. Unlike the PCK, a DAT file is an uncompressed
 * image with no offsets so these have to be figured out
 * manually, usually by splitting the image into equal portions.
 * @param filename Filename of the DAT image.
 * @sa http://www.ufopaedia.org/index.php?title=Image_Formats#SCR_.26_DAT
 */
void SurfaceSet::loadDat(string filename)
{
	// Load file and put pixels in surface
	ifstream imgFile (filename.c_str(), ios::in | ios::binary);
	if (!imgFile)
	{
		throw "Failed to load DAT";
	}
	
	imgFile.seekg(0, ios::end);
	streamoff size = imgFile.tellg();
	imgFile.seekg(0, ios::beg);

	_nframe = (int)size / (_width * _height);

	for (int i = 0; i < _nframe; i++)
	{
		SDL_Rect rect;
		rect.x = 0;
		rect.y = i*_height;
		rect.w = _width;
		rect.h = _height;
		_frames[i] = rect;
	}

	_surface = new Surface(_width, _height * _nframe);
	SDL_Surface *surface = _surface->getSurface();
	
	char value;

	// Lock the surface
	SDL_LockSurface(surface);

	Uint8 *index = (Uint8 *)surface->pixels;

	while (imgFile.read(&value, 1))
	{
		*index = Uint8(value);
		index++;
	}

	if (!imgFile.eof())
	{
		throw "Invalid data from file";
	}

	// Unlock the surface
	SDL_UnlockSurface(surface);

	imgFile.close();
}

/**
 * Returns a particular frame from the image set.
 * @param i Frame to use for size/position.
 * @return Pointer to the set's surface with the respective
 * cropping rectangle set up.
 */
Surface *SurfaceSet::getFrame(int i)
{
	_surface->setCrop(&_frames[i]);
	return _surface;
}

/**
 * Returns the full width of a frame in the set.
 * @return Width in pixels.
 */
int SurfaceSet::getWidth()
{
	return _width;
}

/**
 * Returns the full height of a frame in the set.
 * @return Height in pixels.
 */
int SurfaceSet::getHeight()
{
	return _height;
}

/**
 * Returns the surface stored within the set.
 * @return Pointer to the internal surface.
 */
Surface* SurfaceSet::getSurface()
{
	return _surface;
}