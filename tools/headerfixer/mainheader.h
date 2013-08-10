// ---------------------------------------------------------------------------
//
//   Lily Engine alpha
//
//   Copyright (c) 2012 Clifford Jolly
//     http://expiredpopsicle.com
//     expiredpopsicle@gmail.com
//
// ---------------------------------------------------------------------------
//
//   This software is provided 'as-is', without any express or implied
//   warranty. In no event will the authors be held liable for any
//   damages arising from the use of this software.
//
//   Permission is granted to anyone to use this software for any
//   purpose, including commercial applications, and to alter it and
//   redistribute it freely, subject to the following restrictions:
//
//   1. The origin of this software must not be misrepresented; you must
//      not claim that you wrote the original software. If you use this
//      software in a product, an acknowledgment in the product
//      documentation would be appreciated but is not required.
//
//   2. Altered source versions must be plainly marked as such, and must
//      not be misrepresented as being the original software.
//
//   3. This notice may not be removed or altered from any source
//      distribution.
//
// -------------------------- END HEADER -------------------------------------

const unsigned int mainHeader_len = 1125;
const char mainHeader[] = {
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d,
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d,
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d,
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x0a, 0x0a, 0x20, 0x20, 0x4c,
    0x69, 0x6c, 0x79, 0x20, 0x45, 0x6e, 0x67, 0x69, 0x6e, 0x65, 0x20, 0x61, 0x6c, 0x70, 0x68, 0x61, 0x0a, 0x0a, 0x20, 0x20,
    0x43, 0x6f, 0x70, 0x79, 0x72, 0x69, 0x67, 0x68, 0x74, 0x20, 0x28, 0x63, 0x29, 0x20, 0x32, 0x30, 0x31, 0x32, 0x20, 0x43,
    0x6c, 0x69, 0x66, 0x66, 0x6f, 0x72, 0x64, 0x20, 0x4a, 0x6f, 0x6c, 0x6c, 0x79, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x68, 0x74,
    0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x65, 0x78, 0x70, 0x69, 0x72, 0x65, 0x64, 0x70, 0x6f, 0x70, 0x73, 0x69, 0x63, 0x6c, 0x65,
    0x2e, 0x63, 0x6f, 0x6d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x65, 0x78, 0x70, 0x69, 0x72, 0x65, 0x64, 0x70, 0x6f, 0x70, 0x73,
    0x69, 0x63, 0x6c, 0x65, 0x40, 0x67, 0x6d, 0x61, 0x69, 0x6c, 0x2e, 0x63, 0x6f, 0x6d, 0x0a, 0x0a, 0x2d, 0x2d, 0x2d, 0x2d,
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d,
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d,
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d,
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x0a, 0x0a, 0x20, 0x20, 0x54, 0x68, 0x69, 0x73, 0x20,
    0x73, 0x6f, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x20, 0x69, 0x73, 0x20, 0x70, 0x72, 0x6f, 0x76, 0x69, 0x64, 0x65, 0x64,
    0x20, 0x27, 0x61, 0x73, 0x2d, 0x69, 0x73, 0x27, 0x2c, 0x20, 0x77, 0x69, 0x74, 0x68, 0x6f, 0x75, 0x74, 0x20, 0x61, 0x6e,
    0x79, 0x20, 0x65, 0x78, 0x70, 0x72, 0x65, 0x73, 0x73, 0x20, 0x6f, 0x72, 0x20, 0x69, 0x6d, 0x70, 0x6c, 0x69, 0x65, 0x64,
    0x0a, 0x20, 0x20, 0x77, 0x61, 0x72, 0x72, 0x61, 0x6e, 0x74, 0x79, 0x2e, 0x20, 0x49, 0x6e, 0x20, 0x6e, 0x6f, 0x20, 0x65,
    0x76, 0x65, 0x6e, 0x74, 0x20, 0x77, 0x69, 0x6c, 0x6c, 0x20, 0x74, 0x68, 0x65, 0x20, 0x61, 0x75, 0x74, 0x68, 0x6f, 0x72,
    0x73, 0x20, 0x62, 0x65, 0x20, 0x68, 0x65, 0x6c, 0x64, 0x20, 0x6c, 0x69, 0x61, 0x62, 0x6c, 0x65, 0x20, 0x66, 0x6f, 0x72,
    0x20, 0x61, 0x6e, 0x79, 0x0a, 0x20, 0x20, 0x64, 0x61, 0x6d, 0x61, 0x67, 0x65, 0x73, 0x20, 0x61, 0x72, 0x69, 0x73, 0x69,
    0x6e, 0x67, 0x20, 0x66, 0x72, 0x6f, 0x6d, 0x20, 0x74, 0x68, 0x65, 0x20, 0x75, 0x73, 0x65, 0x20, 0x6f, 0x66, 0x20, 0x74,
    0x68, 0x69, 0x73, 0x20, 0x73, 0x6f, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x2e, 0x0a, 0x0a, 0x20, 0x20, 0x50, 0x65, 0x72,
    0x6d, 0x69, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x20, 0x69, 0x73, 0x20, 0x67, 0x72, 0x61, 0x6e, 0x74, 0x65, 0x64, 0x20, 0x74,
    0x6f, 0x20, 0x61, 0x6e, 0x79, 0x6f, 0x6e, 0x65, 0x20, 0x74, 0x6f, 0x20, 0x75, 0x73, 0x65, 0x20, 0x74, 0x68, 0x69, 0x73,
    0x20, 0x73, 0x6f, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x20, 0x66, 0x6f, 0x72, 0x20, 0x61, 0x6e, 0x79, 0x0a, 0x20, 0x20,
    0x70, 0x75, 0x72, 0x70, 0x6f, 0x73, 0x65, 0x2c, 0x20, 0x69, 0x6e, 0x63, 0x6c, 0x75, 0x64, 0x69, 0x6e, 0x67, 0x20, 0x63,
    0x6f, 0x6d, 0x6d, 0x65, 0x72, 0x63, 0x69, 0x61, 0x6c, 0x20, 0x61, 0x70, 0x70, 0x6c, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6f,
    0x6e, 0x73, 0x2c, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x74, 0x6f, 0x20, 0x61, 0x6c, 0x74, 0x65, 0x72, 0x20, 0x69, 0x74, 0x20,
    0x61, 0x6e, 0x64, 0x0a, 0x20, 0x20, 0x72, 0x65, 0x64, 0x69, 0x73, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x20, 0x69,
    0x74, 0x20, 0x66, 0x72, 0x65, 0x65, 0x6c, 0x79, 0x2c, 0x20, 0x73, 0x75, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x20, 0x74, 0x6f,
    0x20, 0x74, 0x68, 0x65, 0x20, 0x66, 0x6f, 0x6c, 0x6c, 0x6f, 0x77, 0x69, 0x6e, 0x67, 0x20, 0x72, 0x65, 0x73, 0x74, 0x72,
    0x69, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x73, 0x3a, 0x0a, 0x0a, 0x20, 0x20, 0x31, 0x2e, 0x20, 0x54, 0x68, 0x65, 0x20, 0x6f,
    0x72, 0x69, 0x67, 0x69, 0x6e, 0x20, 0x6f, 0x66, 0x20, 0x74, 0x68, 0x69, 0x73, 0x20, 0x73, 0x6f, 0x66, 0x74, 0x77, 0x61,
    0x72, 0x65, 0x20, 0x6d, 0x75, 0x73, 0x74, 0x20, 0x6e, 0x6f, 0x74, 0x20, 0x62, 0x65, 0x20, 0x6d, 0x69, 0x73, 0x72, 0x65,
    0x70, 0x72, 0x65, 0x73, 0x65, 0x6e, 0x74, 0x65, 0x64, 0x3b, 0x20, 0x79, 0x6f, 0x75, 0x20, 0x6d, 0x75, 0x73, 0x74, 0x0a,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x6e, 0x6f, 0x74, 0x20, 0x63, 0x6c, 0x61, 0x69, 0x6d, 0x20, 0x74, 0x68, 0x61, 0x74, 0x20,
    0x79, 0x6f, 0x75, 0x20, 0x77, 0x72, 0x6f, 0x74, 0x65, 0x20, 0x74, 0x68, 0x65, 0x20, 0x6f, 0x72, 0x69, 0x67, 0x69, 0x6e,
    0x61, 0x6c, 0x20, 0x73, 0x6f, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x2e, 0x20, 0x49, 0x66, 0x20, 0x79, 0x6f, 0x75, 0x20,
    0x75, 0x73, 0x65, 0x20, 0x74, 0x68, 0x69, 0x73, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x73, 0x6f, 0x66, 0x74, 0x77, 0x61,
    0x72, 0x65, 0x20, 0x69, 0x6e, 0x20, 0x61, 0x20, 0x70, 0x72, 0x6f, 0x64, 0x75, 0x63, 0x74, 0x2c, 0x20, 0x61, 0x6e, 0x20,
    0x61, 0x63, 0x6b, 0x6e, 0x6f, 0x77, 0x6c, 0x65, 0x64, 0x67, 0x6d, 0x65, 0x6e, 0x74, 0x20, 0x69, 0x6e, 0x20, 0x74, 0x68,
    0x65, 0x20, 0x70, 0x72, 0x6f, 0x64, 0x75, 0x63, 0x74, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x64, 0x6f, 0x63, 0x75, 0x6d,
    0x65, 0x6e, 0x74, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x77, 0x6f, 0x75, 0x6c, 0x64, 0x20, 0x62, 0x65, 0x20, 0x61, 0x70,
    0x70, 0x72, 0x65, 0x63, 0x69, 0x61, 0x74, 0x65, 0x64, 0x20, 0x62, 0x75, 0x74, 0x20, 0x69, 0x73, 0x20, 0x6e, 0x6f, 0x74,
    0x20, 0x72, 0x65, 0x71, 0x75, 0x69, 0x72, 0x65, 0x64, 0x2e, 0x0a, 0x0a, 0x20, 0x20, 0x32, 0x2e, 0x20, 0x41, 0x6c, 0x74,
    0x65, 0x72, 0x65, 0x64, 0x20, 0x73, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x73,
    0x20, 0x6d, 0x75, 0x73, 0x74, 0x20, 0x62, 0x65, 0x20, 0x70, 0x6c, 0x61, 0x69, 0x6e, 0x6c, 0x79, 0x20, 0x6d, 0x61, 0x72,
    0x6b, 0x65, 0x64, 0x20, 0x61, 0x73, 0x20, 0x73, 0x75, 0x63, 0x68, 0x2c, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x6d, 0x75, 0x73,
    0x74, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x6e, 0x6f, 0x74, 0x20, 0x62, 0x65, 0x20, 0x6d, 0x69, 0x73, 0x72, 0x65, 0x70,
    0x72, 0x65, 0x73, 0x65, 0x6e, 0x74, 0x65, 0x64, 0x20, 0x61, 0x73, 0x20, 0x62, 0x65, 0x69, 0x6e, 0x67, 0x20, 0x74, 0x68,
    0x65, 0x20, 0x6f, 0x72, 0x69, 0x67, 0x69, 0x6e, 0x61, 0x6c, 0x20, 0x73, 0x6f, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x2e,
    0x0a, 0x0a, 0x20, 0x20, 0x33, 0x2e, 0x20, 0x54, 0x68, 0x69, 0x73, 0x20, 0x6e, 0x6f, 0x74, 0x69, 0x63, 0x65, 0x20, 0x6d,
    0x61, 0x79, 0x20, 0x6e, 0x6f, 0x74, 0x20, 0x62, 0x65, 0x20, 0x72, 0x65, 0x6d, 0x6f, 0x76, 0x65, 0x64, 0x20, 0x6f, 0x72,
    0x20, 0x61, 0x6c, 0x74, 0x65, 0x72, 0x65, 0x64, 0x20, 0x66, 0x72, 0x6f, 0x6d, 0x20, 0x61, 0x6e, 0x79, 0x20, 0x73, 0x6f,
    0x75, 0x72, 0x63, 0x65, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x64, 0x69, 0x73, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x69,
    0x6f, 0x6e, 0x2e, 0x0a, 0x0a, 0x00,
};
