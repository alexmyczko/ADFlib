/*
 * file_seek_test2.c
 */

#include <stdio.h>
#include <stdlib.h>
#include"adflib.h"


#define NUM_TESTS  1000
#define MAX_ERRORS 10


typedef struct test_file_s {
    char * image_filename;
    char * filename_adf;
    char * filename_local;

    unsigned int len;
} test_file_t;


//const unsigned int ofs_highest_pos_not_using_extension = 35135;  // 0x893f

test_file_t test_file_ofs = {
    .filename_adf = "moon.gif",
    .filename_local = "../Files/testofs_adf/MOON.GIF",
    .len = 173847
};


//const unsigned int ffs_highest_pos_not_using_extension = //39935;  // 0x9bff   ( 512 * 72) ?
//    36863;  // 0x9bff

test_file_t test_file_ffs = {
    .filename_adf = "mod.And.DistantCall",
    .filename_local = "../Files/testffs_adf/mod.And.DistantCall",
    .len = 145360
};


int run_multiple_seek_tests ( test_file_t * test_data );

int test_single_seek ( struct File * const file_adf,
                       FILE * const        file_local,
                       unsigned int        offset );


int main ( int argc, char * argv[] )
{ 
    adfEnvInitDefault();

//	adfSetEnvFct(0,0,MyVer,0);
    int status = 0;

    test_file_ofs.image_filename = argv[1];
    status += run_multiple_seek_tests ( &test_file_ofs );

    test_file_ffs.image_filename = argv[2];
    status += run_multiple_seek_tests ( &test_file_ffs );

    adfEnvCleanUp();

    return status;
}


int run_multiple_seek_tests ( test_file_t * test_data )
{
    printf ( "\n*** Testing multiple file seeking"
             "\n\timage file:\t%s\n\tfilename:\t%s\n\tlocal file:\t%s\n",
             test_data->image_filename,
             test_data->filename_adf,
             test_data->filename_local );

    struct Device * const dev = adfMountDev ( test_data->image_filename, TRUE );
    if ( ! dev ) {
        printf ( "Cannot mount image %s - aborting the test...\n",
                 test_data->image_filename );
        return 1;
    }

    struct Volume * const vol = adfMount ( dev, 0, TRUE );
    if ( ! vol ) {
        printf ( "Cannot mount volume 0 from image %s - aborting the test...\n",
                 test_data->image_filename );
        adfUnMountDev ( dev );
        return 1;
    }
    adfVolumeInfo ( vol );

    int status = 0;
    struct File * const file_adf = adfOpenFile ( vol, test_data->filename_adf, "r" );
    if ( ! file_adf ) {
        fprintf ( stderr, "Cannot open adf file %s - aborting...\n",
                  test_data->filename_adf );
        status = 1;
        goto cleanup;
    }

    FILE * file_local = fopen ( test_data->filename_local, "r" );
    if ( ! file_local ) {
        fprintf ( stderr, "Cannot open local file %s - aborting...\n",
                  test_data->filename_local );
        goto cleanup_adffile;
    }

    for ( int i = 0 ; i < NUM_TESTS && status < MAX_ERRORS ; ++i ) {
        status += test_single_seek ( file_adf, file_local,
                                     random() % test_data->len );
    }

    fclose ( file_local );

cleanup_adffile:
    adfCloseFile ( file_adf );

cleanup:
    adfUnMount ( vol );
    adfUnMountDev ( dev );

    return status;
}


int test_single_seek ( struct File * const file_adf,
                       FILE * const        file_local,
                       unsigned int        offset )
{
    printf ( "  Reading data after seek to position 0x%x (%d)...",
             offset, offset );

    adfFileSeek ( file_adf, offset );

    unsigned char c;
    int n = adfReadFile ( file_adf, 1, &c );

    if ( n != 1 ) {
        printf ( " -> Reading data failed!!!\n" );
        return 1;
    }

    unsigned char expected_value;
    fseek ( file_local, offset, SEEK_SET );
    fread ( &expected_value, 1, 1, file_local );
    
    if ( c != expected_value ) {
        printf ( " -> Incorrect data read:  expected 0x%x != read 0x%x\n",
                 (int) expected_value, (int) c );
        return 1;
    }

    printf ( " -> OK.\n" );
    return 0;
}