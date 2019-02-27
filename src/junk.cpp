
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <algorithm>
using namespace std;

struct MIDI{

    char* ChunkType;
    unsigned int HeaderLength;
    unsigned int Format;
    unsigned int NumberTracks; //ntrks 
    unsigned int Division; //分解能

    MIDI(){
        ChunkType = new char[6];
    }

    void ChunkTypeSet( char* s ){
        fill( ChunkType, ChunkType+6, '\0' );
        for( int i = 0; i < 4; i++ ){
            ChunkType[i] = s[i];
        }
        fill( s, s+4, '\0' );
    }

    void HeaderLengthSet( char* s ){
        HeaderLength = 0;
        for( int i = 0; i<4 ; i++ ){
            HeaderLength *= 0x100;
            HeaderLength += s[i]&0xFF;
        }
    }

    void FormatSet( char* s ){
        Format = s[1];
    }

    void NumberTracksSet( char* s ){
        NumberTracks = 0;
        NumberTracks |= s[0]&0xFF;
        NumberTracks <<= 8;
        NumberTracks |= s[1]&0xFF;
    }

    void DivisionSet( char* s ){
        s[0] &= 0x7f;
        Division = 0;
        Division |= s[0]&0xFF;
        Division <<= 8;
        Division |= s[1]&0xFF;
    }

    void ShowAllProperties(){
        printf( "ChunkType    : %s\n", ChunkType );
        printf( "Format       : %u\n", Format );
        printf( "NumberTracks : %u\n", NumberTracks );
        printf( "Division     : %u\n", Division );
    }

};

int main(){
    fstream ofs;

    ofs.open("../MIDIsample/C5_C6_Cmaj_Cmin_on_MuseScore.mid", ios::in | ios::binary );

    if( !ofs ){
        printf( "\a\a\aerror\n" );
        return -1;
    }else{
        printf( "\aread successfully!\n" );
    }

    char b[256] = {};
    int count = 0;
    MIDI m;
    ofs.read( b, 4 );
    m.ChunkTypeSet( b );
    ofs.read( b, 4 );
    m.HeaderLengthSet( b );
    ofs.read( b, 2 );
    m.FormatSet( b );
    ofs.read( b, 2 );
    m.NumberTracksSet( b );
    ofs.read( b, 2 );
    m.DivisionSet( b );
    m.ShowAllProperties();

    for( int TrackIndex = 0; TrackIndex < m.NumberTracks; TrackIndex++ ){
        ofs.read( b, 4 );
        ofs.read( b, 4 );
        int TrackLeng = 0;
        for( int i = 0; i<4 ; i++ ){
            TrackLeng *= 0x100;
            TrackLeng += b[i]&0xFF;
        }
        int event = 0;
        int dedltatime = 0;
        int velocity,channel,note;
        int s = 0;
        for(int DataIndex = 0; DataIndex < TrackLeng; ){
            /**
             * デルタタイムを読み取る 
             */
            dedltatime = 0;
            ofs.read( (char*)&s, 1 );
            DataIndex++;
            while( s & 0x80 ){
                dedltatime |= s & 0x7F;
                dedltatime <<= 7;
                ofs.read( (char*)&s, 1 );
                DataIndex++;
            }
            dedltatime |= s & 0x7F;
            printf( "deltatime:%d\n", dedltatime );

            /**
             * イベントを読み込む、ランニングステータスによく注意を
             */
            ofs.read( (char*)&s, 1 );
            DataIndex++;
            // 今とったのはイベント？
            if( s & 0x80 ){ 
                event = s;
                ofs.read( (char*)&s, 1 );
                DataIndex++;
            }
            printf( "event    :%02X\n", event );

            // 兎にも角にもイベントの選別
            switch ( event & 0xF0 ){
                case 0x90:
                    // note on
                    channel = event & 0xF;
                    note = s;
                    ofs.read( (char*)&s, 1 );
                    DataIndex++;
                    velocity = s ;
                    printf( "[note on]\n" );
                    printf( "ch  :  %d\n", channel);
                    printf( "nt  :  %2X\n", note);
                    printf( "vl  :  %d\n", velocity );
                    break;

                case 0xB0:
                    // コントロールチェンジ
                    ofs.read( (char*) &s, 1 );
                    DataIndex++;
                    ofs.read( (char*) &s, 1 );
                    DataIndex++;
                    break;

                case 0xC0:
                    // プログラムチェンジ
                    ofs.read( (char*) &s, 1 );
                    DataIndex++;

                // メタイベント・SysExイベント
                case 0xF0 :
                    // メタイベント
                    if( event == 0xFF ){
                        printf("subevent :%02X\n", s );
                        switch ( s ){

                            case 0x21:
                                //ポート指定
                                printf( "Modulation Wheel\n");
                                ofs.read( (char*) &s, 1 );
                                DataIndex++;
                                ofs.read( (char*) &s, 1 );
                                DataIndex++;
                                break;

                            case 0x2F:
                                // トラック終端
                                printf("[End of Track]\n");
                                ofs.read( (char*) &s, 1 );
                                DataIndex++;
                                break;

                            case 0x51:
                                // テンポ
                                printf("[Set Tempo]\n");
                                ofs.read( (char*) &s, 1 );
                                DataIndex++;
                                ofs.read( (char*) &s, 1 );
                                DataIndex++;
                                ofs.read( (char*) &s, 1 );
                                DataIndex++;
                                ofs.read( (char*) &s, 1 );
                                DataIndex++;
                                break;

                            case 0x58:
                                // メトロノーム
                                printf("[Time Signature]\n");
                                ofs.read( (char*) &s, 1 );
                                DataIndex++;
                                ofs.read( (char*) &s, 1 );
                                DataIndex++;
                                ofs.read( (char*) &s, 1 );
                                DataIndex++;
                                ofs.read( (char*) &s, 1 );
                                DataIndex++;
                                ofs.read( (char*) &s, 1 );
                                DataIndex++;
                                break;

                            case 0x59:
                                // 調
                                printf("[Key Signature]\n");
                                ofs.read( (char*) &s, 1 );
                                DataIndex++;
                                ofs.read( (char*) &s, 1 );
                                DataIndex++;
                                ofs.read( (char*) &s, 1 );
                                DataIndex++;
                                break;

                        
                            default:
                                break;
                        }
                    }
                    break;
                
                default:
                    printf( "Unkown\n" );
                    break;
            }
            printf("\n");

        }
        
    }
    printf("\n");

}