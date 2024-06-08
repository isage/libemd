#include <elfio/elfio.hpp>
#include "scestubwriter.h"

using namespace ELFIO;

SceStubWriter::SceStubWriter(ngpImports *imp, bool weak) : StubWriter(imp, weak) {}

SceStubWriter::~SceStubWriter()
{
}

void SceStubWriter::make_head_stub(ngpImportsLib *library, PPAr* ar)
{
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

    section* sceLibstub_sec = writer.sections.add( ".sceLib.stub" );
    sceLibstub_sec->set_type( SHT_PROGBITS );
    sceLibstub_sec->set_flags( SHF_ALLOC );
    sceLibstub_sec->set_addr_align(4);

    section* sceLibgenMark_sec = writer.sections.add( ".sce_libgen_mark" );
    sceLibgenMark_sec->set_type( SHT_PROGBITS );
    sceLibgenMark_sec->set_addr_align(4);
    sceLibgenMark_sec->append_data( "\x18\x00\x00\x00", 4 ); // size + type
    sceLibgenMark_sec->append_data( (char*)&library->NID, 4 ); // lib nid
    sceLibgenMark_sec->append_data( "\x00\x00\x00\x00", 4 );
    sceLibgenMark_sec->append_data( "\x01\x00\x00\x00", 4 );

    uint32_t flags = library->flags;
    if (_weak) flags |= 0x8;

    sceLibgenMark_sec->append_data( (char*)&flags, 4 );
    sceLibgenMark_sec->append_data( "\x00\x00\x00\x00", 4 );

    section* sceImport_sec = writer.sections.add( ".sceImport.rodata" );
    sceImport_sec->set_type( SHT_PROGBITS );
    sceImport_sec->set_flags( SHF_ALLOC );
    sceImport_sec->set_addr_align(4);
    sceImport_sec->append_data( library->name.c_str(), library->name.length() );

    section* rel_sec = writer.sections.add( ".rel.sce_libgen_mark" );
    rel_sec->set_type( SHT_REL );
    rel_sec->set_info( sceLibgenMark_sec->get_index() );
    rel_sec->set_addr_align( 0x4 );
    rel_sec->set_entry_size( writer.get_default_entry_size( SHT_REL ) );

    relocation_section_accessor rela( writer, rel_sec );

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

    rel_sec->set_link( sym_sec->get_index() );

    symbol_section_accessor syma( writer, sym_sec );

    syma.add_symbol(stra, ".text", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, text_sec->get_index() );
    syma.add_symbol(stra, ".data", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, data_sec->get_index() );
    syma.add_symbol(stra, ".bss", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, bss_sec->get_index() );
    syma.add_symbol(stra, ".sceLib.stub", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, sceLibstub_sec->get_index() );

    syma.add_symbol(stra, ".sce_libgen_mark", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, sceLibgenMark_sec->get_index() );
    syma.add_symbol(stra, ".rel.sce_libgen_mark", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, rel_sec->get_index() );
    syma.add_symbol(stra, ".sceImport.rodata", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, sceLibstub_sec->get_index() );

    syma.add_symbol(stra, ".shstrtab", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, 1 );
    syma.add_symbol(stra, ".strtab", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, str_sec->get_index() );
    syma.add_symbol(stra, ".symtab", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, sym_sec->get_index() );

    std::string head_name = fmt::format("_{}_0001_stub_head", library->name);
    syma.add_symbol(stra, head_name.c_str(), 0, 0, STB_GLOBAL, STT_OBJECT, 0,  sceLibgenMark_sec->get_index());

    std::string stub_str_name = fmt::format("_{}_stub_str", library->name);
    Elf_Word sym_to_adjust = syma.add_symbol(stra, stub_str_name.c_str(), 0, 0, STB_GLOBAL, STT_OBJECT, 0,  sceImport_sec->get_index());
    rela.add_entry( 4, sym_to_adjust, (unsigned char)R_ARM_ABS32 );

    std::string inid_name = fmt::format("_INID_{}_{}", library->name.length(), library->name);
    syma.add_symbol(stra, inid_name.c_str(), library->NID, 0, STB_GLOBAL, STT_OBJECT, 0,  SHN_ABS);

    std::string package_name = fmt::format("_sce_package_version_{}", library->name);
    syma.add_symbol(stra, package_name.c_str(), 0x14, 0, STB_LOCAL, STT_OBJECT, 0,  sceLibgenMark_sec->get_index());

    syma.add_symbol(stra, "$d", 0x00000000, 0, STB_LOCAL, STT_NOTYPE, 0,  sceLibgenMark_sec->get_index());
    syma.add_symbol(stra, "$d", 0x00000000, 0, STB_LOCAL, STT_NOTYPE, 0,  sceLibstub_sec->get_index());
    syma.add_symbol(stra, "$d", sceLibgenMark_sec->get_size(), 0, STB_LOCAL, STT_NOTYPE, 0,  sceLibgenMark_sec->get_index());

    syma.arrange_local_symbols( [&]( Elf_Xword first, Elf_Xword second ) {
        rela.swap_symbols( first, second );
    } );

    std::string filename = fmt::format("_{}-0001_head.o", library->name);

    // add stub_head, stub_str and INID_lib to ar symbols
    PPArMember* m = ar->addFile(filename);
    m->addSymbol(head_name);
    m->addSymbol(stub_str_name);
    m->addSymbol(inid_name);
    writer.save( m->data );

}

void SceStubWriter::make_nid_stub(ngpImportsLib *library, PPAr* ar)
{
 // add all NID_ (lib, then funcs, then vars) to ar symbols
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
    syma.add_symbol(stra, ".shstrtab", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, 1 );
    syma.add_symbol(stra, ".strtab", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, str_sec->get_index() );
    syma.add_symbol(stra, ".symtab", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, sym_sec->get_index() );

    std::string lib_nid_name = fmt::format("_NID_{}", library->name);
    syma.add_symbol(stra, lib_nid_name.c_str(), library->NID, 0, STB_GLOBAL, STT_OBJECT, 0,  SHN_ABS);

    for (auto& func: library->functions)
    {
        std::string func_nid_name = fmt::format("_NID_{}", func->name);
        syma.add_symbol(stra, func_nid_name.c_str(), func->NID, 0, STB_GLOBAL, STT_OBJECT, 0,  SHN_ABS);
    }

    for (auto& var: library->variables)
    {
        std::string var_nid_name = fmt::format("_NID_{}", var->name);
        syma.add_symbol(stra, var_nid_name.c_str(), var->NID, 0, STB_GLOBAL, STT_OBJECT, 0,  SHN_ABS);
    }

    syma.arrange_local_symbols();

    std::string filename = fmt::format("_{}-0001_NIDS.o", library->name);

    // add stub_head, stub_str and INID_lib to ar symbols
    PPArMember* m = ar->addFile(filename);
    m->addSymbol(lib_nid_name);
    for (auto& func: library->functions)
    {
        std::string func_nid_name = fmt::format("_NID_{}", func->name);
        m->addSymbol(func_nid_name);
    }

    for (auto& var: library->variables)
    {
        std::string var_nid_name = fmt::format("_NID_{}", var->name);
        m->addSymbol(var_nid_name);
    }

    writer.save( m->data );
}

void SceStubWriter::make_func_stubs(ngpImportsLib *library, PPAr* ar)
{
    for (auto& func: library->functions)
    {
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

        section* sceFStub_sec = writer.sections.add( ".sceFStub.rodata" );
        sceFStub_sec->set_type( SHT_PROGBITS );
        sceFStub_sec->set_flags( SHF_ALLOC );
        sceFStub_sec->set_addr_align(4);
        sceFStub_sec->append_data( "\0\0\0\0", 4 );

        section* sceRefs_sec = writer.sections.add( ".sceRefs.rodata" );
        sceRefs_sec->set_type( SHT_PROGBITS );
        sceRefs_sec->set_flags( SHF_ALLOC );
        sceRefs_sec->set_addr_align(4);
        sceRefs_sec->append_data( "\0\0\0\0", 4 );
        sceRefs_sec->append_data( "\0\0\0\0", 4 );
        sceRefs_sec->append_data( "\0\0\0\0", 4 );
        sceRefs_sec->append_data( "\0\0\0\0", 4 );
        sceRefs_sec->append_data( "\0\0\0\0", 4 );

        section* sceFNID_sec = writer.sections.add( ".sceFNID.rodata" );
        sceFNID_sec->set_type( SHT_PROGBITS );
        sceFNID_sec->set_flags( SHF_ALLOC );
        sceFNID_sec->set_addr_align(4);
        sceFNID_sec->append_data( "\0\0\0\0", 4 );

        std::string sceStub_name = fmt::format(".sceStub.text.{}.{}", library->name, func->name);

        section* sceStub_sec = writer.sections.add( sceStub_name.c_str() );
        sceStub_sec->set_type( SHT_PROGBITS );
        sceStub_sec->set_flags( SHF_ALLOC | SHF_EXECINSTR );
        sceStub_sec->set_addr_align(4);

        char stub[] = {
            '\x00', '\x00', '\xE0', '\xE3', // mvn     r0, #0
            '\x1E', '\xFF', '\x2F', '\xE1', // bx      lr
            '\x00', '\x00', '\xA0', '\xE1', // mov     r0, r0
            '\x00', '\x00', '\x00', '\x00'  // andeq   r0, r0, r0
        };
        sceStub_sec->set_data( stub, sizeof( stub ) );

        section* sceLibgenMark_sec = writer.sections.add( ".sce_libgen_mark" );
        sceLibgenMark_sec->set_type( SHT_PROGBITS );
        sceLibgenMark_sec->set_addr_align(4);
        sceLibgenMark_sec->append_data( "\x14\x01\x00\x00", 4 );
        sceLibgenMark_sec->append_data( "\x00\x00\x00\x00", 4 );
        sceLibgenMark_sec->append_data( "\x00\x00\x00\x00", 4 );
        sceLibgenMark_sec->append_data( (char*)&func->NID, 4 );
        sceLibgenMark_sec->append_data( "\x00\x00\x00\x00", 4 );

        section* rel_sec = writer.sections.add( ".rel.sce_libgen_mark" );
        rel_sec->set_type( SHT_REL );
        rel_sec->set_info( sceLibgenMark_sec->get_index() );
        rel_sec->set_addr_align( 0x4 );
        rel_sec->set_entry_size( writer.get_default_entry_size( SHT_REL ) );

        relocation_section_accessor rela( writer, rel_sec );

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

        rel_sec->set_link( sym_sec->get_index() );

        symbol_section_accessor syma( writer, sym_sec );

        syma.add_symbol(stra, ".text", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, text_sec->get_index() );
        syma.add_symbol(stra, ".data", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, data_sec->get_index() );
        syma.add_symbol(stra, ".bss", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, bss_sec->get_index() );
        syma.add_symbol(stra, ".sceFStub.rodata", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, sceFStub_sec->get_index() );
        syma.add_symbol(stra, ".sceRefs.rodata", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, sceRefs_sec->get_index() );
        syma.add_symbol(stra, ".sceFNID.rodata", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, sceFNID_sec->get_index() );
        syma.add_symbol(stra, sceStub_name.c_str(), 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, sceStub_sec->get_index() );
        syma.add_symbol(stra, ".sce_libgen_mark", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, sceLibgenMark_sec->get_index() );
        syma.add_symbol(stra, ".rel.sce_libgen_mark", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, rel_sec->get_index() );

        syma.add_symbol(stra, ".shstrtab", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, 1 );
        syma.add_symbol(stra, ".strtab", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, str_sec->get_index() );
        syma.add_symbol(stra, ".symtab", 0x00000000, 0, STB_LOCAL, STT_SECTION, 0, sym_sec->get_index() );

        std::string inid_name = fmt::format("_INID_{}_{}__{}_{}", library->name.length(), library->name, func->name.length(), func->name);
        syma.add_symbol(stra, inid_name.c_str(), func->NID, 0, STB_GLOBAL, STT_OBJECT, 0,  SHN_ABS);

        syma.add_symbol(stra, "$a", 0x00000000, 0, STB_LOCAL, STT_NOTYPE, 0,  sceStub_sec->get_index());

        std::string head_name = fmt::format("_{}_0001_stub_head", library->name);

        Elf_Word sym_to_adjust = syma.add_symbol(stra, head_name.c_str(), 0x00000000, 0, STB_GLOBAL, STT_NOTYPE, 0, 0);
        rela.add_entry( 4, sym_to_adjust, (unsigned char)R_ARM_ABS32 );

        sym_to_adjust = syma.add_symbol(stra, func->name.c_str(), 0x00000000, 16, STB_GLOBAL, STT_FUNC, 0, sceStub_sec->get_index() );
        rela.add_entry( 8, sym_to_adjust, (unsigned char)R_ARM_ABS32 );

        std::string nid_name = fmt::format("_NID_{}", func->name);
        sym_to_adjust = syma.add_symbol(stra, nid_name.c_str(), 0x00000000, 16, STB_GLOBAL, STT_NOTYPE, 0, 0 );
        rela.add_entry( 0x10, sym_to_adjust, (unsigned char)R_ARM_ABS32 );

        syma.arrange_local_symbols( [&]( Elf_Xword first, Elf_Xword second ) {
            rela.swap_symbols( first, second );
        } );

        // add NID_<func> and <func> to ar symbols

        std::string filename = fmt::format("_{}-0001_F00_{:08x}.o", library->name, func->NID);

        PPArMember* m = ar->addFile(filename);
        m->addSymbol(inid_name);
        m->addSymbol(func->name);
        writer.save( m->data );

        return;
    }
}

void SceStubWriter::make_var_stubs(ngpImportsLib *library, PPAr* ar)
{

}


void SceStubWriter::make_stub()
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

        make_func_stubs(library, &ar);
        make_var_stubs(library, &ar);
        make_nid_stub(library, &ar);
        make_head_stub(library, &ar);

        ar.save();
      }
    }


}

