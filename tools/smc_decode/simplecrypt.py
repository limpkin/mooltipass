# -*- coding: utf-8 -*-
# Copyright (c) 2016, Sunguk Lee
# Original C++ Copyright (c) 2011, Andre Somers
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the Rathenau Instituut, Andre Somers nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE # DISCLAIMED. IN NO EVENT SHALL ANDRE SOMERS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR #######; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
from __future__ import unicode_literals
import sys
from enum import Enum, IntEnum
import base64
from struct import pack, unpack
import random
import zlib
import hashlib

__all__ = ['CompressionMode', 'IntegrityProtectionMode', 'CryptoFlag',
           'SimpleCryptException', 'SimpleCrypt']

CRC_TABLE = (0x0000, 0x1081, 0x2102, 0x3183,
             0x4204, 0x5285, 0x6306, 0x7387,
             0x8408, 0x9489, 0xa50a, 0xb58b,
             0xc60c, 0xd68d, 0xe70e, 0xf78f)


class CompressionMode(Enum):
    CompressionAuto = 0
    CompressionAlways = 1
    CompressionNever = 2


class IntegrityProtectionMode(Enum):
    ProtectionNone = 0
    ProtectionChecksum = 1
    ProtectionHash = 2


class CryptoFlag(IntEnum):
    CryptoFlagNone = 0
    CryptoFlagCompression = 0x01
    CryptoFlagChecksum = 0x02
    CryptoFlagHash = 0x04


def compress(buf):
    # maximum compression
    return pack('>I', len(buf)) + zlib.compress(buf, 9)

def uncompress(buf):
    if len(buf) < 4:
        return b''
    expected_size = max(unpack('>I', buf[:4])[0], 1)
    ret = zlib.decompress(buf[4:])
    return ret

def uint8(s):
    if type(s) == str:
        return ord(s)
    return s

def byte(s):
    if str == bytes:
        return s
    return s.encode('latin1')

def checksum(buf):
    crc = 0xffff
    for x in buf:
        x = uint8(x)
        crc = ((crc >> 4) & 0x0fff) ^ CRC_TABLE[((crc ^ x) & 15)]
        x >>= 4
        crc = ((crc >> 4) & 0x0fff) ^ CRC_TABLE[((crc ^ x) & 15)]
    return ~crc & 0xffff


class SimpleCryptException(Exception):
    pass


class SimpleCrypt(object):

    def __init__(self, key):
        self._key = key
        self._compression_mode = CompressionMode.CompressionAuto
        self._protection_mode = IntegrityProtectionMode.ProtectionChecksum
        self._key_parts = []
        self.split_key()

    def set_key(self, key):
        self._key = key
        self.split_key()

    def split_key(self):
        self._key_parts = []
        for x in range(8):
            part = self._key & 0xffffffffffffffff
            for y in range(x, 0, -1):
                part >>= 8
            part &= 0xff
            self._key_parts.append(part)
        #print(self._key_parts)

    def encrypt_to_bytes(self, text):
        if sys.version_info.major == 3:
            if type(text) == str:
                text = text.encode('utf-8')
            if type(text) != bytes:
                return b''

        if not len(self._key_parts):
            raise SimpleCryptException('No key set')
        ba = text
        flags = CryptoFlag.CryptoFlagNone.value

        if self._compression_mode == CompressionMode.CompressionAlways:
            ba = compress(ba)
            flags |= CryptoFlag.CryptoFlagCompression.value
        elif self._compression_mode == CompressionMode.CompressionAuto:
            compressed = compress(ba)
            if len(compressed) < len(ba):
                ba = compressed
                flags |= CryptoFlag.CryptoFlagCompression.value

        if self._protection_mode == IntegrityProtectionMode.ProtectionChecksum:
            flags |= CryptoFlag.CryptoFlagChecksum.value
            integrity_protection = pack('>H', checksum(ba))
        elif self._protection_mode == IntegrityProtectionMode.ProtectionHash:
            flags |= CryptoFlag.CryptoFlagHash.value
            integrity_protection = hashlib.sha1(ba).digest()

        random_char = byte(chr(random.randint(0, 0xff)))
        ba = random_char + integrity_protection + ba

        pos = 0
        last = 0

        cnt = len(ba)
        while pos < cnt:
            curr = uint8(ba[pos])
            new = curr ^ self._key_parts[pos % 8] ^ last
            ba = ba[:pos] + byte(chr(new)) + ba[pos + 1:]
            last = new
            pos += 1
        result = [
            # version for future updates to algorithm
            chr(0x3).encode('latin1'),
            # encryption flags
            chr(flags).encode('latin1'),
        ]

        return b''.join(result) + ba

    def encrypt_to_string(self, text):
        cypher = self.encrypt_to_bytes(text)
        return base64.b64encode(cypher)

    def decrypt_to_string(self, cypher):
        plain = self.decrypt_to_bytes(cypher)
        return plain.decode('utf-8')

    def decrypt_to_bytes(self, cypher):
        if sys.version_info.major == 3:
            if type(cypher) == str:
                cypher = base64.b64decode(cypher.encode('latin1'))
        else:
            if type(cypher) == unicode:
                cypher = base64.b64decode(cypher.encode('latin1'))
        if type(cypher) != bytes:
            return b''

        if not len(self._key_parts):
            # no key set
            self.last_error = 1
            return b''

        if len(cypher) < 3:
            return b''

        ba = cypher
        version = uint8(ba[0])

        # we only work with version 3
        if version != 3:
            raise SimpleCryptException('Invalid version or not a cyphertext')

        flags = uint8(ba[1])

        ba = ba[2:]
        pos = 0
        cnt = len(ba)
        last = 0

        while pos < cnt:
            curr = uint8(ba[pos])
            new = curr ^ last ^ self._key_parts[pos % 8]
            ba = ba[:pos] + byte(chr(new)) + ba[pos + 1:]
            last = curr
            pos += 1

        # chop off the random number at the start
        ba = ba[1:]

        integrity_ok = True

        if flags & CryptoFlag.CryptoFlagChecksum:
            if len(ba) < 2:
                raise SimpleCryptException('Integrity failed')
            stored_checksum = unpack('>H', ba[:2])[0]
            ba = ba[2:]
            integrity_ok = checksum(ba) == stored_checksum
        elif flags & CryptoFlag.CryptoFlagHash:
            if len(ba) < 20:
                raise SimpleCryptException('Integrity failed')
            stored_hash = be[:20]
            ba = ba[20:]
            integrity_ok = hashlib.sha1(ba).digest() == stored_hash

        if not integrity_ok:
            raise SimpleCryptException('Integrity failed')

        if flags & CryptoFlag.CryptoFlagCompression:
            ba = uncompress(ba)

        return ba
