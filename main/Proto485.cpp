#include <Arduino.h>
#include "Proto485.h"
//#define DEBUG485
#define LMAXPKT 20
#define A 0
#define L 1
#define D 2
#define C 3
#define S 4
Proto485::Proto485(Stream *stream, int txenablepin, bool testreg) {
    _txenablepin=txenablepin;
    _stream=stream;
    _testreg=testreg;
}

void Proto485::ProcessaDatiSeriali(unsigned char c) {
  static byte numerobytesricevuti=0,bytesricevuti[LMAXPKT],prossimodato=A,lunghezza,comando,sum;
  static unsigned long tultimodatoricevuto;
  #ifdef DEBUG485
  Serial.print("c=");
  Serial.print(c,HEX);
  Serial.print(" prox=");
  Serial.println(prossimodato);
  #endif
  if(millis()-tultimodatoricevuto > 300) {prossimodato=A; };
  tultimodatoricevuto=millis();
  if(prossimodato==A && c=='A') {prossimodato=C; numerobytesricevuti=0; return;}
  if(prossimodato==C) {comando=c; prossimodato=L; sum=c; return;}
  if(prossimodato==L) {
    sum+=c;
    lunghezza=c;
    prossimodato=D; 
    if(lunghezza>LMAXPKT) prossimodato=A;
    if(lunghezza==0) {
      prossimodato=S;
    }
    #ifdef DEBUG485
    Serial.print("next D");
    Serial.print(" l=");
    Serial.println(lunghezza);
    #endif
    return;
  }
  if(prossimodato==D) {
    sum+=c;
    bytesricevuti[numerobytesricevuti++]=c;
    #ifdef DEBUG485
    Serial.print("DD l=");
    Serial.print(lunghezza);
    Serial.print(" br=");
    Serial.println(numerobytesricevuti);
    #endif
    if(numerobytesricevuti==lunghezza) prossimodato=S;
    return;
  }
  if(prossimodato==S) {
    if(c==sum) {
      if (cbElaboraComando) this->cbElaboraComando(comando,bytesricevuti,lunghezza);
      prossimodato=A;
      #ifdef DEBUG485
      Serial.print("next A");
      #endif
    } else {
      #ifdef DEBUG485
      Serial.print("somma errata");
      #endif
    }
  }
}

void Proto485::Tx(char cmd, byte len, const char* b) {
    byte sum=(byte)cmd+len;
    //for(byte r=0;r<len;r++) sum+=b[r];
    digitalWrite(_txenablepin, HIGH);
    _stream->write('A');
    _stream->write(cmd);
    _stream->write(len);
    for (byte f=0;f<len;f++) {
      _stream->write(b[f]);
      sum+=b[f];
    }
    _stream->write(sum);
    #ifdef DEBUG485
      Serial.println();
      Serial.print("pkt: A:");
      Serial.print(cmd,DEC);
      Serial.print(":");
      Serial.print(len,DEC);
      Serial.print(":");
      for (byte f=0;f<len;f++) {
        Serial.print(b[f],DEC);
        Serial.print(":");
      }
      Serial.println(sum,DEC);
      #endif
      
    if (_testreg) while (!(UCSR0A & _BV(TXC0)));
    digitalWrite(_txenablepin, LOW);
}
