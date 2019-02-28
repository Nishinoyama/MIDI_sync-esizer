
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <algorithm>
#include <windows.h>

using namespace std;
char b[256] = {};
int s = 0;


int DataIndex = 0;
HMIDIOUT hmo;
fstream ofs;

int ReadVarialbe( fstream& ofs ){
    int var = 0;
    ofs.read( (char*)&s, 1 );
    DataIndex++;
    while( s & 0x80 ){
        var |= s & 0x7F;
        var <<= 7;
        ofs.read( (char*)&s, 1 );
        DataIndex++;
    }
    var |= s & 0x7F;
    return var;
}

class MIDI{

public :

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
        // printf( "ChunkType    : %s\n", ChunkType );
        // printf( "Format       : %u\n", Format );
        // printf( "NumberTracks : %u\n", NumberTracks );
        // printf( "Division     : %u\n", Division );
    }

    void ReadHeader( fstream& ofs ){

        ofs.read( b, 4 );
        ChunkTypeSet( b );
        ofs.read( b, 4 );
        HeaderLengthSet( b );
        ofs.read( b, 2 );
        FormatSet( b );
        ofs.read( b, 2 );
        NumberTracksSet( b );
        ofs.read( b, 2 );
        DivisionSet( b );
    }

    void EventOccurrer( int event , int data2, fstream& ofs ){
        int data1 = 0, data3 = 0;
        switch ( event & 0xF0 ){

            case 0x80: // note off
                data1 = event & 0xF;
                ofs.read( (char*)&s, 1 );
                DataIndex++;
                data3 = s ;
                // printf( "[note off]\n" );
                // printf( "ch  :  %d\n", data1);
                // printf( "nt  :  %2X\n", data2);
                // printf( "vl  :  %d\n", data3 );
                midiOutShortMsg(hmo, data3<<16 | data2<<8 | event );
                break;

            case 0x90: // note on
                data1 = event & 0xF;
                ofs.read( (char*)&s, 1 );
                DataIndex++;
                data3 = s ;
                // printf( "[note on]\n" );
                // printf( "ch  :  %d\n", data1);
                // printf( "nt  :  %2X\n", data2);
                // printf( "vl  :  %d\n", data3 );
                // printf( "%X\n", data3<<16 | data2<<8 | event );
                midiOutShortMsg(hmo, data3<<16 | data2<<8 | event );
                break;
            
            case 0xA0: // ポリフォニックキープレッシャー
                data1 = event & 0xF;
                data2 = s;
                ofs.read( (char*)&s, 1 );
                DataIndex++;
                data3 = s ;
                // printf( "[Polyphonic Key Pressure]\n" );
                // printf( "ch  :  %d\n", data1);
                // printf( "nt  :  %2X\n", data2);
                // printf( "vl  :  %d\n", data3 );
                midiOutShortMsg(hmo, data3<<16 | data2<<8 | event );
                break;

            case 0xB0: // コントロールチェンジ
                // printf( "[Controll Change]\n");
                ofs.read( (char*) &s, 1 );
                DataIndex++;
                data3 = s;
                midiOutShortMsg(hmo, data3<<16 | data2<<8 | event );
                break;

            case 0xC0: // プログラムチェンジ
                // printf( "[Program Change]\n");
                midiOutShortMsg(hmo, data2<<8 | event );
                break;

            case 0xE0: // Pitch
                // printf( "[Pitch]\n");
                ofs.read( (char*) &s, 1 );
                DataIndex++;
                data3 = s;
                midiOutShortMsg(hmo, data3<<16 | data2<<8 | event );
                break;
            case 0xF0: // メタイベント・SysExイベント
                // メタイベント
                if( event == 0xF7 || event == 0xF0 ){
                    // printf( "[SysExEvent]\n" );
                    ofs.read( b , data2 );
                    DataIndex++;
                }
                else
                if( event == 0xFF ){
                    // printf("subevent :%02X\n", data2 );
                    data3 = ReadVarialbe( ofs );
                    switch ( data2 ){

                        case 0x21:
                            //ポート指定
                            // printf( "Modulation Wheel\n");
                            ofs.read( (char*) &s, 1 );
                            DataIndex++;
                            break;

                        case 0x2F:
                            // トラック終端
                            // printf("[End of Track]\n");
                            break;

                        case 0x51:
                            // テンポ
                            // printf("[Set Tempo]\n");
                            ofs.read( (char*) &s, 1 );
                            DataIndex++;
                            ofs.read( (char*) &s, 1 );
                            DataIndex++;
                            ofs.read( (char*) &s, 1 );
                            DataIndex++;
                            break;

                        case 0x58:
                            // メトロノーム
                            // printf("[Time Signature]\n");
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
                            // printf("[Key Signature]\n");
                            ofs.read( (char*) &s, 1 );
                            DataIndex++;
                            ofs.read( (char*) &s, 1 );
                            DataIndex++;
                            break;

                        default:
                            // printf( "[text]\n" );
                            ofs.read( b, data3 );
                            DataIndex += data3;
                            break;
                    }
                }
        }
    }

};



int main(){

    ofs.open("../MIDIsample/C5_C6_Cmaj_Cmin_on_MuseScore.mid", ios::in | ios::binary );
    if (midiOutOpen(&hmo, MIDI_MAPPER, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR){
        // printf( "\a\a\aerror\n" );
        return -1;
    }

    if( !ofs ){
        // printf( "\a\a\aerror\n" );
        return -1;
    }else{
        // printf( "\aread successfully!\n" );
    }

    int count = 0;
    MIDI m;

    m.ReadHeader( ofs );

    for( int TrackIndex = 0; TrackIndex < m.NumberTracks; TrackIndex++ ){
        ofs.read( b, 4 );
        ofs.read( b, 4 );
        int TrackLeng = 0;
        for( int i = 0; i<4 ; i++ ){
            TrackLeng *= 0x100;
            TrackLeng += b[i]&0xFF;
        }
        int event = 0;
        int deltatime = 0;
        for( DataIndex = 0; DataIndex < TrackLeng; ){
            /**
             * デルタタイムを読み取る 
             */
            deltatime = ReadVarialbe( ofs );
            // printf( "deltatime:%X\n", deltatime );
            Sleep( deltatime );

            /**
             * イベントを読み込む、ランニングステータスによく注意を
             */
            ofs.read( (char*)&s, 1 );
            DataIndex++;
            // 今とったのはイベント？（ランニングステータスならイベントではない）
            if( s & 0x80 ){ 
                // イベント更新
                event = s;
                ofs.read( (char*)&s, 1 );
                DataIndex++;
            }
            // printf( "event    :%02X\n", event );

            // イベントを走る
            m.EventOccurrer( event, (int)s , ofs );
            // printf("\n");

        }

    }
    // printf("\n");

    midiOutClose( hmo );

    return 0;

}
