#include <check.h>
#include <stdlib.h>

#ifndef _WIN32
#include <unistd.h>   // for unlink()
#endif

#include "adflib.h"
//#include "adf_util.h"
#include "test_util.h"

#define TEST_VERBOSITY 0

typedef struct test_data_s {
    struct AdfDevice * device;
//    struct AdfVolume * vol;
    char *             adfname;
    char *             volname;
    uint8_t            fstype;   // 0 - OFS, 1 - FFS
    unsigned           nVolumeBlocks;
    char *             openMode;  // "w" or "a"
    unsigned char *    buffer;
    unsigned           bufsize;
    unsigned           truncsize;
} test_data_t;


void setup ( test_data_t * const tdata );
void teardown ( test_data_t * const tdata );


START_TEST ( test_check_framework )
{
    ck_assert ( 1 );
}
END_TEST


void test_file_truncate ( test_data_t * const tdata )
{
    struct AdfDevice * const device = tdata->device;
    ck_assert_ptr_nonnull ( device );


    ///
    /// mount the test volume
    ///

    // mount the test volume
    struct AdfVolume * vol = // tdata->vol =
        adfMount ( device, 0, FALSE );
    ck_assert_ptr_nonnull ( vol );

    // check it is an empty floppy disk
    unsigned free_blocks_before = adfCountFreeBlocks ( vol );
    ck_assert_int_eq ( tdata->nVolumeBlocks, free_blocks_before );
    int nentries = adfDirCountEntries ( vol, vol->curDirPtr );
    ck_assert_int_eq ( 0, nentries ); 


    ///
    /// create a new file
    ///

    char filename[] = "testfile.tmp";
    struct AdfFile * file = adfFileOpen ( vol, filename, tdata->openMode );
    ck_assert_ptr_nonnull ( file );
    adfFileClose ( file );

    // reset volume state (remount)
    adfUnMount ( vol );
    vol = // tdata->vol =
        adfMount ( device, 0, FALSE );

    // verify free blocks
    const unsigned file_blocks_used_by_empty_file = 1;
    ck_assert_int_eq ( free_blocks_before - file_blocks_used_by_empty_file,
                       adfCountFreeBlocks ( vol ) );

    // verify the number of entries
    ck_assert_int_eq ( 1, adfDirCountEntries ( vol, vol->curDirPtr ) );

    // verify file information (meta-data)
    file = adfFileOpen ( vol, filename, "r" );
    ck_assert_ptr_nonnull ( file );
    ck_assert_uint_eq ( 0, file->fileHdr->byteSize );
    ck_assert_uint_eq ( 0, file->pos );
    ck_assert_int_eq ( 0, file->posInExtBlk );
    //ck_assert_int_eq ( 0, file->posInDataBlk );
    ck_assert_int_eq ( 0, file->nDataBlock );
    ck_assert_int_eq ( adfEndOfFile ( file ), TRUE );
    adfFileClose ( file );


    ///
    /// write data to the created above, empty file
    ///
    
    // open for writing
    file = adfFileOpen ( vol, filename, "w" );
    ck_assert_uint_eq ( 0, file->fileHdr->byteSize );
    ck_assert_int_eq ( file->fileHdr->firstData, 0 );
    ck_assert_uint_eq ( 0, file->pos );
    ck_assert_int_eq ( 0, file->posInExtBlk );
    //ck_assert_int_eq ( 0, file->posInDataBlk );
    ck_assert_int_eq ( 0, file->nDataBlock );
    ck_assert_int_eq ( adfEndOfFile ( file ), TRUE );

    // write data buffer to the file
    const unsigned bufsize = tdata->bufsize;
    const unsigned char * const buffer = tdata->buffer;
    unsigned bytes_written = adfFileWrite ( file, bufsize, buffer );
    ck_assert_int_eq ( bufsize, bytes_written );
    adfFileClose ( file );

    // reset volume state (remount)
    adfUnMount ( vol );
    vol = //tdata->vol =
        adfMount ( device, 0, FALSE );

    // verify free blocks
    //ck_assert_int_eq ( free_blocks_before - file_blocks_used_by_empty_file - 1,
    //                   adfCountFreeBlocks ( vol ) );
    //int expected_free_blocks = free_blocks_before - file_blocks_used_by_empty_file - 1;
    const unsigned free_blocks_with_file_expected =
        free_blocks_before - filesize2blocks ( bufsize, vol->datablockSize );
    const unsigned free_blocks_with_file = adfCountFreeBlocks ( vol );
    ck_assert_msg (
        free_blocks_with_file == free_blocks_with_file_expected,
        "Free blocks after writing the file incorrect: %u (should be %u), bufsize %u",
        free_blocks_with_file, free_blocks_with_file_expected, bufsize );

    // verify the number of entries
    ck_assert_int_eq ( 1, adfDirCountEntries ( vol, vol->curDirPtr ) );

    // verify file information (meta-data)
    file = adfFileOpen ( vol, filename, "r" );
    ck_assert_uint_eq ( bufsize, file->fileHdr->byteSize );
    ck_assert_int_gt ( file->fileHdr->firstData, 0 );
    ck_assert_uint_eq ( 0, file->pos );
    ck_assert_int_eq ( 0, file->posInExtBlk );
    //ck_assert_int_eq ( 0, file->posInDataBlk );
    ck_assert_int_eq ( 1, file->nDataBlock );
    ck_assert_int_eq ( adfEndOfFile ( file ), FALSE );    
    adfFileClose ( file );

    //
    // test truncating the file
    // 
    const unsigned truncsize = tdata->truncsize;
    
    // truncate the file
    file = adfFileOpen ( vol, filename, "w" );
    ck_assert_ptr_nonnull ( file );

#if ( TEST_VERBOSITY >= 1 )
    printf ( "Testing with: bufsize %u, truncsize %u, dblock size %u\n",
             bufsize, truncsize, file->volume->datablockSize );
    fflush(stdout);
#endif
    //ck_assert_int_eq ( RC_OK, adfFileTruncate ( file, truncsize ) );
    ck_assert_msg ( adfFileTruncate ( file, truncsize ) == RC_OK,
        "adfFileTruncate failed, bufsize %u, truncsize %u",
        bufsize, truncsize );

    // check file metadata before closing the file
    //ck_assert_uint_eq ( truncsize, file->pos );
    ck_assert_msg (  truncsize == file->pos,
                     " truncsize %u != file->pos %u, bufsize %u, truncsize %u",
                     truncsize, file->pos, bufsize, truncsize );
    ck_assert_uint_eq ( truncsize, file->fileHdr->byteSize );
    adfFileClose ( file );

    // reset volume state (remount)
    adfUnMount ( vol );
    vol = adfMount ( device, 0, FALSE );
    
    // check volume metadata after truncating
    ck_assert_int_eq ( 1, adfDirCountEntries ( vol, vol->curDirPtr ) );

    const unsigned free_blocks_file_truncated_expected =
        free_blocks_before - filesize2blocks ( truncsize, vol->datablockSize );
    const unsigned free_blocks_file_truncated = adfCountFreeBlocks ( vol );
    ck_assert_msg (
        free_blocks_file_truncated == free_blocks_file_truncated_expected,
        "Free blocks after truncating incorrect: %u (should be %u), bufsize %u, truncsize %u",
        free_blocks_file_truncated, free_blocks_file_truncated_expected, bufsize, truncsize );
    
    // reread the truncated file
    file = adfFileOpen ( vol, filename, "r" );
    ck_assert_ptr_nonnull ( file );
    unsigned char * rbuf = malloc ( truncsize + 1 );
    ck_assert_ptr_nonnull ( rbuf );
    unsigned bytes_read = adfFileRead ( file, truncsize + 1, rbuf );
    free ( rbuf );
    rbuf = NULL;

    ck_assert_int_eq ( truncsize, bytes_read );
    ck_assert_uint_eq ( truncsize, file->pos );

    //ck_assert_int_eq ( 0, file->posInExtBlk );  // TODO
    //ck_assert_int_eq ( 1, file->posInDataBlk );

    /*unsigned expected_nDataBlock =
        //( ( bufsize - 1 ) / vol->datablockSize ) + 1;
        //filesize2datablocks ( bufsize, vol->datablockSize ) + 1;
        pos2datablockIndex ( (unsigned) max ( (int) truncsize - 1, 0 ), vol->datablockSize ) + 1;
    //ck_assert_int_eq ( expected_nDataBlock, file->nDataBlock );
   ck_assert_msg ( file->nDataBlock == expected_nDataBlock,
                    "file->nDataBlock %d == expected %d, truncsize %u",
                    file->nDataBlock, expected_nDataBlock, truncsize );
    */
    ck_assert_int_eq ( adfEndOfFile ( file ), TRUE );
    adfFileClose ( file );

    // verify data of the truncated file

    // this verifies up to the size of the buffer
    unsigned commonSize = min ( bufsize, truncsize );
    ck_assert_msg ( verify_file_data ( vol, filename, buffer, commonSize, 10 ) == 0,
                    "Data verification failed, bufsize %u (0x%x), "
                    "truncsize %u (0x%x), commonsize %u (0x%x)",
                    bufsize, bufsize, truncsize, truncsize, commonSize, commonSize );

    // if enlarging the file... 
    if ( truncsize > bufsize ) {
        // ...  must also check the zeroed part

#if ( TEST_VERBOSITY >= 1 )
        printf ("Checking the enlarged (zero-filled) part\n");
        fflush (stdout);
#endif
        file = adfFileOpen ( vol, filename, "r" );
        ck_assert_ptr_nonnull ( file );
        adfFileSeek ( file, bufsize );
        for ( unsigned i = 0 ; i < truncsize - bufsize ; ++i ) {
            uint8_t data;
            ck_assert_int_eq ( adfFileRead ( file, 1, &data ), 1 );
            ck_assert_msg ( data == 0,
                            "Non-zero data in enlarged/zeroed part, offset 0x%x (%u),"
                            " bufsize %u (0x%x), truncsize %u (0x%x)\n",
                            bufsize + i, bufsize + i,
                            bufsize, bufsize, truncsize, truncsize );
        }
        adfFileClose ( file ); 
    }
    
    // umount volume
    adfUnMount ( vol );
}


static const unsigned buflen[] = {
    1, 2, 256,
    487, 488, 489,
    511, 512, 513,
    970, 974, 975, 976, 977, 978,
    1022, 1023, 1024, 1025,
    2047, 2048, 2049, 2050,
    4095, 4096, 4097,
    10000, 20000, 35000, 35130,
    35136,
    35137,    // the 1st requiring an ext. block (OFS)
    35138,
    36000, 36864,
    36865,    // the 1st requiring an ext. block (FFS)
    37000, 37380, 40000,
    60000, 69784, 69785, 69796, 69800, 70000,
    100000,
    //200000, 512000, 800000,
    820000
};
static const unsigned buflensize = sizeof ( buflen ) / sizeof ( unsigned );


static const unsigned truncsizes[] = {
    0, 1, 2, 256,
    487, 488, 489,
    511, 512, 513,
    970, 974, 975, 976, 977, 978,
    1022, 1023, 1024, 1025,
    2047, 2048, 2049, 2050,
    4095, 4096, 4097,
    //10000, 20000, 35000, 35130,
    35136,
    35137,    // the 1st requiring an ext. block (OFS)
    35138,
    36000, 36864,
    36865,    // the 1st requiring an ext. block (FFS)
    37000, 37380, 40000,
    60000, 69784, 69785, 69796, 69800, 70000,
    100000,
    //200000, 512000, 800000,
    //820000
};
static const unsigned ntruncsizes = sizeof ( truncsizes ) / sizeof ( unsigned );


START_TEST ( test_file_truncate_ofs )
{
    test_data_t test_data = {
        .adfname = "test_file_truncate_ofs.adf",
        .volname = "Test_file_truncate_ofs",
        .fstype  = 0,          // OFS
        .openMode = "w",
        .nVolumeBlocks = 1756
    };
    for ( unsigned i = 0 ; i < buflensize ; ++i ) {
        test_data.bufsize = buflen[i];
        for ( unsigned j = 0 ; j < ntruncsizes ; ++j ) {
            test_data.truncsize = truncsizes[j];
            //if ( test_data.bufsize < test_data.truncsize )
            //    continue;
            setup ( &test_data );
            test_file_truncate ( &test_data );
            teardown ( &test_data );
        }
    }
}

START_TEST ( test_file_truncate_ffs )
{
    test_data_t test_data = {
        .adfname = "test_file_truncate_ffs.adf",
        .volname = "Test_file_truncate_ffs",
        .fstype  = 1,          // FFS
        .openMode = "w",
        .nVolumeBlocks = 1756
    };
    for ( unsigned i = 0 ; i < buflensize ; ++i ) {
        test_data.bufsize = buflen[i];
        for ( unsigned j = 0 ; j < ntruncsizes ; ++j ) {
            test_data.truncsize = truncsizes[j];
            //if ( test_data.bufsize < test_data.truncsize )
            //    continue;
            setup ( &test_data );
            test_file_truncate ( &test_data );
            teardown ( &test_data );
        }
    }
}


Suite * adflib_suite ( void )
{
    Suite * s = suite_create ( "adflib" );
    
    TCase * tc = tcase_create ( "check framework" );
    tcase_add_test ( tc, test_check_framework );
    suite_add_tcase ( s, tc );

    tc = tcase_create ( "adflib test_file_truncate_ofs" );
    tcase_add_test ( tc, test_file_truncate_ofs );
    tcase_set_timeout ( tc, 300 );
    suite_add_tcase ( s, tc );

    tc = tcase_create ( "adflib test_file_truncate_ffs" );
    //tcase_add_checked_fixture ( tc, setup_ffs, teardown_ffs );
    tcase_add_test ( tc, test_file_truncate_ffs );
    tcase_set_timeout ( tc, 300 );
    suite_add_tcase ( s, tc );

    return s;
}


int main ( void )
{
    Suite * s = adflib_suite();
    SRunner * sr = srunner_create ( s );

    adfEnvInitDefault();
    srunner_run_all ( sr, CK_VERBOSE ); //CK_NORMAL );
    adfEnvCleanUp();

    int number_failed = srunner_ntests_failed ( sr );
    srunner_free ( sr );
    return ( number_failed == 0 ) ?
        EXIT_SUCCESS :
        EXIT_FAILURE;
}


void setup ( test_data_t * const tdata )
{
    tdata->device = adfCreateDumpDevice ( tdata->adfname, 80, 2, 11 );
    if ( ! tdata->device ) {       
        //return;
        exit(1);
    }
    if ( adfCreateFlop ( tdata->device, tdata->volname, tdata->fstype ) != RC_OK ) {
        fprintf ( stderr, "adfCreateFlop error creating volume: %s\n",
                  tdata->volname );
        exit(1);
    }

    //tdata->vol = adfMount ( tdata->device, 0, FALSE );
    //if ( ! tdata->vol )
    //    return;
    //    exit(1);
    tdata->buffer = malloc ( tdata->bufsize );
    if ( ! tdata->buffer )
        exit(1);
    //pattern_AMIGAMIG ( tdata->buffer, tdata->bufsize );
    pattern_random ( tdata->buffer, tdata->bufsize );
}


void teardown ( test_data_t * const tdata )
{
    free ( tdata->buffer );
    tdata->buffer = NULL;

    //adfUnMount ( tdata->vol );
    adfUnMountDev ( tdata->device );
    if ( unlink ( tdata->adfname ) != 0 )
        perror("error deleting the image");
}
