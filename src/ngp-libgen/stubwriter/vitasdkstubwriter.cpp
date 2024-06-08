#include <elfio/elfio.hpp>
#include <pipiar.h>
#include "vitasdkstubwriter.h"
#include <fmt/core.h>

using namespace ELFIO;

VitasdkStubWriter::VitasdkStubWriter(ngpImports *imp, bool weak) : StubWriter(imp, weak) {}

VitasdkStubWriter::~VitasdkStubWriter() {}

void VitasdkStubWriter::make_stub()
{
    for (auto& module : _imp->modules)
    {
      for (auto& library : module->libs)
      {
        // generate stub. name it
        std::string stubname;

        if (library->stubname.empty())
        {
            if (_weak)
                stubname = fmt::format("lib{}_stub_weak.a", library->name);
            else
                stubname = fmt::format("lib{}_stub.a", library->name);
        }
        else
        {
            if (_weak)
                stubname = fmt::format("{}_weak.a", library->stubname);
            else
                stubname = fmt::format("{}.a", library->stubname);
        }

        PPAr ar(stubname);

        // vitasdk has just .o per func/var

        for (auto& function: library->functions)
        {
            std::string filename = fmt::format("{}_{}_{}.o", module->name, library->name, function->name);

            elfio writer;

            writer.create( ELFCLASS32, ELFDATA2LSB );
            writer.set_os_abi( ELFOSABI_NONE );
            writer.set_type( ET_REL );
            writer.set_machine( EM_ARM );
            writer.set_flags( 0x5000000 );
            writer.set_segment_entry_size(0);

            section* text_sec = writer.sections.add( ".text" );
            text_sec->set_type( SHT_PROGBITS );
            text_sec->set_flags( SHF_ALLOC | SHF_EXECINSTR );
            text_sec->set_addr_align( 0x1 );

            section* data_sec = writer.sections.add( ".data" );
            data_sec->set_type( SHT_PROGBITS );
            data_sec->set_flags( SHF_WRITE | SHF_ALLOC );
            data_sec->set_addr_align( 0x1 );

            section* bss_sec = writer.sections.add( ".bss" );
            bss_sec->set_type( SHT_NOBITS );
            bss_sec->set_flags( SHF_WRITE | SHF_ALLOC );
            bss_sec->set_addr_align( 0x1 );

            section* fstub_sec = writer.sections.add( fmt::format(".vitalink.fstubs.{}",library->name) );
            fstub_sec->set_type( SHT_PROGBITS );
            fstub_sec->set_flags( SHF_ALLOC | SHF_EXECINSTR );
            fstub_sec->set_addr_align(16);

            fstub_sec->append_data( "\x00\x00\x00\x00", 4 ); // TODO: attr + weak/nonweak
            fstub_sec->append_data( (const char*)&library->NID, 4 );
            fstub_sec->append_data( (const char*)&function->NID, 4 );
            fstub_sec->append_data( "\x00\x00\x00\x00", 4 ); // align data to 16

            section* str_sec = writer.sections.add( ".strtab" );
            str_sec->set_type( SHT_STRTAB );
            str_sec->set_addr_align( 0x1 );

            string_section_accessor stra( str_sec );

            section* sym_sec = writer.sections.add( ".symtab" );
            sym_sec->set_type( SHT_SYMTAB );
            sym_sec->set_info( 1 );
            sym_sec->set_addr_align( 0x4 );
            sym_sec->set_entry_size( writer.get_default_entry_size( SHT_SYMTAB ) );
            sym_sec->set_link( str_sec->get_index() );


            symbol_section_accessor syma( writer, sym_sec );

            syma.add_symbol(stra, ".text", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, text_sec->get_index() );
            syma.add_symbol(stra, ".data", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, data_sec->get_index() );
            syma.add_symbol(stra, ".bss", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, bss_sec->get_index() );

            syma.add_symbol(stra, fmt::format(".vitalink.fstubs.{}",library->name).c_str(), 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, fstub_sec->get_index() );


            syma.add_symbol(stra, function->name.c_str(),0x877181ed, 0, STB_GLOBAL, STT_FUNC, 0, fstub_sec->get_index());
            syma.add_symbol(stra, "$d", 0x00000000, 0, STB_LOCAL, STT_NOTYPE, 0,  fstub_sec->get_index());
            syma.add_symbol(stra, "$a", 0x0000000c, 0, STB_LOCAL, STT_NOTYPE, 0,  fstub_sec->get_index());
            syma.arrange_local_symbols();

            PPArMember* m = ar.addFile(filename);
            std::cout << m->filename << std::endl;
            m->addSymbol(function->name);
            writer.save( m->data );
        }

        for (auto& variable: library->variables)
        {
            std::string filename = fmt::format("{}_{}_{}.o", module->name, library->name, variable->name);

            elfio writer;

            writer.create( ELFCLASS32, ELFDATA2LSB );
            writer.set_os_abi( ELFOSABI_NONE );
            writer.set_type( ET_REL );
            writer.set_machine( EM_ARM );
            writer.set_flags( 0x5000000 );
            writer.set_segment_entry_size(0);

            section* text_sec = writer.sections.add( ".text" );
            text_sec->set_type( SHT_PROGBITS );
            text_sec->set_flags( SHF_ALLOC | SHF_EXECINSTR );
            text_sec->set_addr_align( 0x1 );

            section* data_sec = writer.sections.add( ".data" );
            data_sec->set_type( SHT_PROGBITS );
            data_sec->set_flags( SHF_WRITE | SHF_ALLOC );
            data_sec->set_addr_align( 0x1 );

            section* bss_sec = writer.sections.add( ".bss" );
            bss_sec->set_type( SHT_NOBITS );
            bss_sec->set_flags( SHF_WRITE | SHF_ALLOC );
            bss_sec->set_addr_align( 0x1 );

            section* fstub_sec = writer.sections.add( fmt::format(".vitalink.vstubs.{}",library->name) );
            fstub_sec->set_type( SHT_PROGBITS );
            fstub_sec->set_flags( SHF_ALLOC | SHF_EXECINSTR );
            fstub_sec->set_addr_align(16);

            uint32_t flags = library->flags;
            if (_weak) flags |= 0x8;

            fstub_sec->append_data( (char*)&flags, 4 );
            fstub_sec->append_data( (const char*)&library->NID, 4 );
            fstub_sec->append_data( (const char*)&variable->NID, 4 );
            fstub_sec->append_data( "\x00\x00\x00\x00", 4 ); // align data to 16

            section* str_sec = writer.sections.add( ".strtab" );
            str_sec->set_type( SHT_STRTAB );
            str_sec->set_addr_align( 0x1 );

            string_section_accessor stra( str_sec );

            section* sym_sec = writer.sections.add( ".symtab" );
            sym_sec->set_type( SHT_SYMTAB );
            sym_sec->set_info( 1 );
            sym_sec->set_addr_align( 0x4 );
            sym_sec->set_entry_size( writer.get_default_entry_size( SHT_SYMTAB ) );
            sym_sec->set_link( str_sec->get_index() );


            symbol_section_accessor syma( writer, sym_sec );

            syma.add_symbol(stra, ".text", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, text_sec->get_index() );
            syma.add_symbol(stra, ".data", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, data_sec->get_index() );
            syma.add_symbol(stra, ".bss", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, bss_sec->get_index() );

            syma.add_symbol(stra, fmt::format(".vitalink.vstubs.{}",library->name).c_str(), 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, fstub_sec->get_index() );


            syma.add_symbol(stra, variable->name.c_str(),0x877181ed, 0, STB_GLOBAL, STT_FUNC, 0, fstub_sec->get_index());
            syma.add_symbol(stra, "$d", 0x00000000, 0, STB_LOCAL, STT_NOTYPE, 0,  fstub_sec->get_index());
            syma.add_symbol(stra, "$a", 0x0000000c, 0, STB_LOCAL, STT_NOTYPE, 0,  fstub_sec->get_index());
            syma.arrange_local_symbols();

            PPArMember* m = ar.addFile(filename);
            m->addSymbol(variable->name);
            writer.save( m->data );

        }
        ar.save();
      }
    }
}
