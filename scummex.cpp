/* ScummEX - Viewer for Scumm data files
 * Copyright (C) 2003 Adrien Mercier
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header: /Users/sev/projects/sc/s/scummvm/scummex/scummex.cpp,v 1.18 2003/09/22 23:43:29 fingolfin Exp $
 *
 */

#include <stdio.h>
#include "file.h"
#include "scummex.h"
#include "descumm.h"

ScummEX::ScummEX() {
    _image = new Image();
    _sound = new Sound();
    _resource = new Resource();
}

void ScummEX::getFileType(const char *filename) {
	uint32 tag;
	_encbyte = 0;
	char buf[256];
	
	if (_input.isOpen()) {
		_input.close();
	}
		
	_input.open(filename, 1, _encbyte);

	sprintf(buf, "ScummEX - %s", filename);
	_gui->SetTitle(buf);

	_gui->EnableToolbarTool(ID_Close);
	_gui->EnableToolbarTool(ID_FileInfo);

	_input.read(&tag, 4);

	switch (tag) {
		case MKID('LB83'):
		case MKID('LABN'):
		case MKID('RNAM'):
		case MKID('LECF'):
		case MKID('ANIM'):
		case MKID('SOU '):
			_input.seek(0, SEEK_SET);
			_resource->searchBlocks(_blockTable, _input);
			return;

	}

	tag = 0;
	_input.read(&tag, 2);

	switch(tag) {
		case 21040: // OR
		case 20306: // RO
		case 20050: // RN
			_input.seek(0, SEEK_SET);
			_resource->searchOldBlocks(_blockTable, _input);
			return;

	}
	
	_input.close();
	_encbyte = 0x69;
	_input.open(filename, 1, _encbyte);
	_input.read(&tag, 4);

	switch (tag) {
		case MKID('RNAM'):
			_input.seek(0, SEEK_SET);
			_resource->searchBlocks(_blockTable, _input);
			return;

		case MKID('LECF'):
			_input.seek(0, SEEK_SET);
			_resource->searchBlocks(_blockTable, _input);
			return;
	}

	tag = 0;
	_input.read(&tag, 2);

	switch(tag) {
		case 17740: // LE
			_input.seek(0, SEEK_SET);
			_resource->searchOldBlocks(_blockTable, _input);
			return;
	}

	_gui->SetTitle("ScummEX");
	_gui->DisableToolbarTool(ID_Close);
	_gui->DisableToolbarTool(ID_FileInfo);
	_gui->DisplayDialog("Unknown file type!", "Error");
}

void ScummEX::Descumm() {
	
	_input.seek(_blockTable[_blockId].offset, SEEK_SET);
	_scummVersion = _gui->getScummVersionDialog();
	if (_scummVersion != 0) {
		DeScumm(_input, _blockTable[_blockId].blockSize, _scummVersion);
	}
}

void ScummEX::FileDump(const char *filename) {

	uint32 size = 0, tot_size = 0;
	char fbuf2[2048];

	_output.open(filename, 2, 0);
	_input.seek(_blockTable[_blockId].offset, SEEK_SET);

	while (tot_size < _blockTable[_blockId].blockSize) {
		size = _input.read(fbuf2, 1);
		tot_size += size;
		_output.write(fbuf2, 1);
	}
	_output.close();
}

void ScummEX::fileView() {
	int j, i;
	char *text;
	char title[256];
	char buf[256];
	byte* data;
	byte* dataorg;
	int len, offset, nlines, bytes_per_line;
	byte c;
	
	len = _blockTable[_blockId].blockSize;
	offset = _blockTable[_blockId].offset;
	bytes_per_line = 16;
	nlines = len / bytes_per_line;
	
	text = (char *) calloc(((16 + (3*bytes_per_line) + (bytes_per_line/4) + bytes_per_line) * nlines) + 256, 1);

	if (text == NULL)
		return;

	sprintf(title, "%s Block at offset %d", _blockTable[_blockId].blockName, _blockTable[_blockId].offset);
	
	_input.seek(_blockTable[_blockId].offset, SEEK_SET);

	dataorg = data = (byte*) malloc(_blockTable[_blockId].blockSize);

	if (data == NULL)
		return;

	_input.read(data, _blockTable[_blockId].blockSize);

	while (len >= bytes_per_line) {
		sprintf(buf, "%06X: ", offset);
		strcat(text, buf);
		for (i = 0; i < bytes_per_line; i++) {
			sprintf(buf, "%02X ", data[i]);
			strcat(text, buf);
			if (i % 4 == 3) {
				sprintf(buf, " ");
				strcat(text, buf);
			}
		}
		sprintf(buf, " |");
		strcat(text, buf);
		for (i = 0; i < bytes_per_line; i++) {
			c = data[i];
			if (c < 32 || c >= 127)
				c = '.';
			sprintf(buf, "%c", c);
			strcat(text, buf);
		}
		sprintf(buf, "|\n");
		strcat(text, buf);
		data += bytes_per_line;
		len -= bytes_per_line;
		offset += bytes_per_line;
	}

	if (len <= 0) 
		return;

	sprintf(buf, "%06X: ", offset);
	strcat(text, buf);
	for (i = 0; i < len; i++) {
		sprintf(buf, "%02X ", data[i]);
		strcat(text, buf);
		if (i % 4 == 3) {
			sprintf(buf, " ");
			strcat(text, buf);
		}
	}

	for (; i < bytes_per_line; i++) {
		sprintf(buf, "   ");
		strcat(text, buf);
			if (i % 4 == 3) {
				sprintf(buf, " ");
				strcat(text, buf);
			}
	}
	sprintf(buf, " |");
	strcat(text, buf);
	for (i = 0; i < len; i++) {
		c = data[i];
		if (c < 32 || c >= 127)
			c = '.';
		sprintf(buf, "%c", c);
		strcat(text, buf);
	}
	for (; i < bytes_per_line; i++) {
		sprintf(buf, " ");
		strcat(text, buf);
	}
	sprintf(buf, "|\n");
	strcat(text, buf);

	free(dataorg);
	
	_gui->DisplayViewer(title, 660, 480, text);

	free(text);

}

void ScummEX::iMUSEPlay()
{
	_sound->playiMUSE(_input, _blockTable, _blockId, _input);
}

void ScummEX::iMUSEDump(const char *filename)
{
	_output.open(filename, 2, 0);

	_sound->playiMUSE(_input, _blockTable, _blockId, _output, 1);
}

void ScummEX::SOUPlay()
{
	_sound->playSOU(_blockTable, _input, _blockId, _input);
}

void ScummEX::paletteDraw()
{
	_image = new Image();
	_image->drawPalette(_blockTable, _blockId, _input);
}

void ScummEX::bgDraw()
{
	_image = new Image();
	_image->drawBG(_input, _blockTable, _blockId);
}

void ScummEX::SmushFrameDraw()
{
	_image = new Image();
	_image->drawSmushFrame(_blockTable, _blockId, _input);
}

void ScummEX::objectDraw()
{
	_image = new Image();
	_image->drawObject(_input, _blockTable, _blockId);
}

void ScummEX::boxesDraw()
{
	_image = new Image();
	_image->drawBoxes(_blockTable, _blockId, _input);
}

void ScummEX::boxesDrawOverlay()
{
	_image = new Image();
	_image->drawBoxes(_blockTable, _blockId, _input, 0);
}

void ScummEX::FileInfo() {
	int fsize;
	fsize = _input.size();
	_gui->FileInfoDialog(fsize, _encbyte);
}
	
void ScummEX::UpdateInfosFromTree(int blockid) {
	_blockId = blockid;

	_gui->updateLabel("TypeLabel", "Type", (uint32)_blockTable[blockid].blockType);
	_gui->updateLabel("OffsetLabel", "Offset", _blockTable[blockid].offset);
	_gui->updateLabel("SizeLabel", "Size", _blockTable[blockid].blockSize);
	_gui->updateLabel("DescriptionLabel", "Description", (uint32)_blockTable[blockid].blockDescription);

	switch(_blockTable[blockid].blockTypeID) {
		case MAXS:
			_gui->updateLabel("SpecLabel1", "Number of variables", _blockTable[blockid].variables);
			_gui->updateLabel("SpecLabel2", "Number of bit variables", _blockTable[blockid].bitVariables);
			_gui->updateLabel("SpecLabel3", "Number of local objects", _blockTable[blockid].localObjects);
			_gui->updateLabel("SpecLabel4", "Number of arrays", _blockTable[blockid].arrays);
			_gui->updateLabel("SpecLabel5", "Number of character sets", _blockTable[blockid].characters);
			_gui->updateLabel("SpecLabel6", "Number of inventory objects", _blockTable[blockid].invObjects);
			break;

		case LB83:
		case LABN:
			_gui->updateLabel("SpecLabel1", "Number of files", _blockTable[blockid].numFiles);
			break;

		case RMHD:
		case HD:
			_gui->updateLabel("SpecLabel1", "Room Width", _blockTable[blockid].width);
			_gui->updateLabel("SpecLabel2", "Room Height", _blockTable[blockid].height);
			_gui->updateLabel("SpecLabel3", "Number of Objects", _blockTable[blockid].localObjects);
			break;

		case TRNS:
			_gui->updateLabel("SpecLabel1", "Transparent Color", _blockTable[blockid].trans);
			break;
			
		case DROO:
		case DSCR:
		case DSOU:
		case DCOS:
		case DCHR:
		case DOBJ:
			_gui->updateLabel("SpecLabel1", "Number of items", _blockTable[blockid].numFiles);
			break;

		case cus2:
			_gui->SetButton(_blockTable[blockid].blockTypeID);
			break;
			
		case COMP:
			_gui->updateLabel("SpecLabel1", "Number of samples", _blockTable[blockid].numFiles);
			break;

		case CLUT:
		case APAL:
		case PA:
		case NPAL:
		case AHDR:
			_gui->SetButton(_blockTable[blockid].blockTypeID);
			break;

		case RMIM:
		case BM:
		case IMAG:
			_gui->SetButton(_blockTable[blockid].blockTypeID);
			break;

		case OBIM:
			if (_blockTable[blockid+3].blockTypeID == 35) {
				_gui->SetButton(_blockTable[blockid].blockTypeID);
			}
			break;

		case IMHD:
			_gui->updateLabel("SpecLabel1", "Room Width", _blockTable[blockid].width);
			_gui->updateLabel("SpecLabel2", "Room Height", _blockTable[blockid].height);
			_gui->updateLabel("SpecLabel3", "Number of Images", _blockTable[blockid].numFiles);
			break;
			
		case Crea:
			_gui->updateLabel("SpecLabel1", "Sample Rate", _blockTable[blockid].variables);
			_gui->SetButton(_blockTable[blockid].blockTypeID);
			break;

		case LSCR:
		case SCRP:
		case ENCD:
		case EXCD:
		case VERB:
		case LS:
		case SC:
		case EN:
		case EX:
		case OC:
			_gui->SetButton(_blockTable[blockid].blockTypeID);
			break;

		case FOBJ:
			_gui->updateLabel("SpecLabel1", "Frame Width", _blockTable[blockid].width);
			_gui->updateLabel("SpecLabel2", "Frame Height", _blockTable[blockid].height);
			_gui->updateLabel("SpecLabel3", "Codec", _blockTable[blockid].variables);
			_gui->SetButton(_blockTable[blockid].blockTypeID);
			break;

		case BOXD:
			_gui->updateLabel("SpecLabel1", "No. of Boxes", _blockTable[blockid].numFiles);
			_gui->SetButton(_blockTable[blockid].blockTypeID);
			break;
	}
}

/*int main(int argc, char *argv[])
{
	ScummEX *_scummex;
	_scummex = new ScummEX();

    return 0;

}*/
