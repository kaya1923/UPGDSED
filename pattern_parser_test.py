#!/usr/bin/env python3
"""
Test script to demonstrate pattern parsing functionality for UPGDSED
"""

import re

def parse_test_mcp():
    """Parse the existing test MCP file"""
    
    # The content that's currently in our test file
    content = '''// Test MCP file with hex patterns
// Function: MyTestFunction

00401000 55                    push    ebp
00401001 89E5                  mov     ebp, esp
00401003 8B4508                mov     eax, [ebp+8]
00401006 8B550C                mov     edx, [ebp+12]

// Pattern data for ImgpValidateImageHash
// Build number: 22621

//  First pattern bytes:
//  48 89 5C 24 08 
//  48 89 74 24 10 
//  57 
//  48 83 EC 20 
//  48 8B F9 
//  48 8B FA 
//  48 8B D9 
//  48 8B E9 
//  48 89 7C 24 18 

// Expected hex pattern:
48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8B F9 48 8B FA 48 8B D9 48 8B E9 48 89 7C 24 18'''

    print("Parsing test MCP file:")
    print("=" * 40)
    
    # Extract the hex pattern from the last line
    lines = content.split('\n')
    pattern_line = None
    
    for line in reversed(lines):  # Look from bottom up 
        if '48 89 5C 24 08 48 89 74 24 10' in line:
            pattern_line = line
            break
    
    if pattern_line:
        print("Found pattern line:")
        print(pattern_line)
        
        # Extract hex values
        hex_values = re.findall(r'[0-9a-fA-F]{2}', pattern_line)
        if len(hex_values) >= 10:
            bytes_array = [int(h, 16) for h in hex_values[:15]]  # First 15 bytes
            
            print(f"\nExtracted {len(bytes_array)} bytes:")
            print("unsigned char pattern[] = { ", end="")
            for i, byte_val in enumerate(bytes_array):
                if i > 0:
                    print(", ", end="")
                print(f"0x{byte_val:02X}", end="")
                if (i + 1) % 8 == 0 and i < len(bytes_array) - 1:
                    print(",")
            print(" };")
            
            # Show hex in readable format
            hex_display = ' '.join([f'{b:02X}' for b in bytes_array[:10]])
            print(f"\nHex dump (first 10): {hex_display}")
        else:
            print("Not enough hex values found")
    else:
        print("No pattern data found in test file")

if __name__ == "__main__":
    parse_test_mcp()