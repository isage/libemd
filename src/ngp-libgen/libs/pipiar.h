/*

Copyright 2024 Epifanov Ivan (Cat)

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

/*
    GNU/Sys-V ar format writer.

    Code is meh and retarded.

    While the base format is somewhat simple
    (except using strings for everything, and padding them with spaces),
    numerous additions make it ugh to work with:

    Problem one: long filenames (easy)
        So, overtime, someone decided that 16 symbols isn't gonna do it for filenames,
        so they invented special "file" with long file names.
        Now, all short filenames must be in the form "/<offset>",
        where offset is a decimal offset into said special file.
        Moreover (in gnu variant), filenames are terminated with /
        (despite also being separated by linefeed).
        This requires constructing such special "file" with filenames
        prior to writing files, remembering offsets into filename table
        and assigning them to short filenames.

    Problem two: symbol lookup (medium/hard)
        Next issue arised when static libs became too big.
        To speed up linking yet another someone invented yet another special "file"
        Now, this one contains two tables:
        First is a table of offsets into ar archive itself, but in big-endian, because fuck you
        Second is a table of symbol names
        Now, since first table requires offsets into archive, instead of, i dunno, just an index of file
        you have to either:
            1. calculate whole layout of archive before writing, so you know where each file header will land
               this also includes requirement that every file header should land on even byte boundary.
            2. calculate symbol "file" size (and for that you need to loop through files to get symbols),
               write dummy data in place of symbol file, and then, after writing all files (including filename "file")
               (and remembering their offsets) go back and write symbol "file" with those offsets

*/

#ifndef __PIPIAR_H__
#define __PIPIAR_H__

#include <arpa/inet.h>
#include <cstdint>
#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// fill with default values
typedef struct
{
  char id[16]    = {'\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
                    '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20'};
  char time[12]  = {'\x30', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20'};
  char oid[6]    = {'\x30', '\x20', '\x20', '\x20', '\x20', '\x20'};
  char gid[6]    = {'\x30', '\x20', '\x20', '\x20', '\x20', '\x20'};
  char mode[8]   = {'\x31', '\x30', '\x30', '\x36', '\x34', '\x34', '\x20', '\x20'};
  char size[10]  = {'\x30', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20'};
  char ending[2] = {'\x60', '\x0A'};
} PPArMemberHeader;

typedef struct
{
  uint32_t offset;
  std::string name;
} PPArSymbol;

class PPArMember
{
public:
  PPArMember(std::string &filename)
      : filename(filename)
  {
  }
  ~PPArMember() { }

  void addSymbol(std::string &name)
  {
    symbols.push_back(name);
  }

  std::string filename;
  uint32_t index  = 0;
  uint32_t offset = 0;
  std::stringstream data;
  std::vector<std::string> symbols;
};

class PPAr
{
public:
  PPAr(std::string &filename)
      : _filename(filename)
  {
  }

  ~PPAr()
  {
    for (auto &member : _members)
    {
      delete member;
    }
  }

  PPArMember *addFile(std::string &filename)
  {
    PPArMember *newMember = new PPArMember(filename);
    _members.push_back(newMember);
    return newMember;
  }

  int save()
  {

    std::ofstream stream;
    stream.open(_filename.c_str(), std::ios::out | std::ios::binary);
    if (!stream)
    {
      return -1;
    }

    // write header
    stream.write("!<arch>\n", 8);

    _reserveSymbols(stream);
    _align(stream);

    _writeFilenames(stream);
    _align(stream);

    _writeFiles(stream);
    // each file write already aligns

    _writeSymbols(stream);

    stream.close();
    return 0;
  }

private:
  int _reserveSymbols(std::ofstream &stream)
  {
    uint32_t _symbolTableSize = sizeof(PPArMemberHeader);
    _symbolTableSize += sizeof(uint32_t);

    for (auto &member : _members)
    {
      for (auto &sym : member->symbols)
      {
        _symbolTableSize += sizeof(uint32_t);
        _symbolTableSize += sym.length() + 1;
      }
    }

    for (uint32_t i = 0; i < _symbolTableSize; i++)
      stream.put('\x00');

    return 0;
  }

  int _writeSymbols(std::ofstream &stream)
  {
    std::vector<PPArSymbol> symbols;
    uint32_t size = 4;

    for (auto &member : _members)
    {
      for (auto &sym : member->symbols)
      {
        symbols.push_back({member->offset, sym});
        size += sizeof(uint32_t) + sym.length() + 1;
      }
    }

    // We have to go back, Marty!
    stream.seekp(8);
    PPArMemberHeader header;
    strncpy(header.id, "/", 1);

    std::string sizestr = std::to_string(size);
    strncpy(header.size, sizestr.c_str(), sizestr.length());

    stream.write(reinterpret_cast<char *>(&header), sizeof(header));

    uint32_t cnt = htonl(symbols.size());
    stream.write(reinterpret_cast<char *>(&cnt), sizeof(cnt));

    for (auto &sym : symbols)
    {
      uint32_t off = htonl(sym.offset);
      stream.write(reinterpret_cast<char *>(&off), sizeof(uint32_t));
    }

    for (auto &sym : symbols)
    {
      stream.write(sym.name.c_str(), sym.name.length() + 1);
    }

    return 0;
  }

  int _writeFilenames(std::ofstream &stream)
  {
    PPArMemberHeader header;
    strncpy(header.id, "//", 2);

    uint32_t size = 0;
    for (auto &member : _members)
    {
      size += member->filename.length() + 2;
    }

    std::string sizestr = std::to_string(size);
    strncpy(header.size, sizestr.c_str(), sizestr.length());

    stream.write(reinterpret_cast<char *>(&header), sizeof(header));

    uint32_t offset = stream.tellp();
    uint32_t pos    = 0;

    for (auto &member : _members)
    {
      stream.write(member->filename.c_str(), member->filename.length());
      stream.write("/\n", 2);
      member->index = pos;
      pos           = ((uint32_t)stream.tellp()) - offset;
    }

    return 0;
  }

  int _writeFiles(std::ofstream &stream)
  {
    for (auto &member : _members)
    {
      member->offset = stream.tellp();
      PPArMemberHeader header;

      std::string index = "/" + std::to_string(member->index);
      strncpy(header.id, index.c_str(), index.length());

      std::string size = std::to_string(member->data.str().length());
      strncpy(header.size, size.c_str(), size.length());

      stream.write(reinterpret_cast<char *>(&header), sizeof(header));

      stream.write(member->data.str().c_str(), member->data.str().length());

      _align(stream);
    }

    return 0;
  }

  void _align(std::ofstream &stream)
  {
    while (stream.tellp() % 2 != 0)
      stream.put('\n');
  }

  std::vector<PPArMember *> _members;
  std::string _filename;
};

#endif
