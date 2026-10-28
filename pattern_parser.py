#!/usr/bin/env python3
"""
Simple UPGDSED Pattern Parser for Ghidra MCP Files
This script helps extract patterns from Ghidra MCP files for the UPGDSED project.
"""

import argparse
import re
import os

def parse_mcp_file(filepath):
    """Parse a Ghidra MCP file to extract byte patterns"""
    
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
            
        # Look for hexadecimal byte patterns (0x48, 0x8B, 0xC4 format)
        pattern_bytes = []
        
        # Find all patterns that look like hex values
        hex_matches = re.findall(r'0x[0-9a-fA-F]{2}', content)
        
        if hex_matches:
            # Parse the bytes and convert to integers
            try:
                pattern_bytes = [int(match, 16) for match in hex_matches]
            except Exception as e:
                print(f"Error parsing bytes: {e}")
                
        return pattern_bytes
        
    except Exception as e:
        print(f"Error reading file {filepath}: {e}")
        return []

def main():
    parser = argparse.ArgumentParser(description='Extract byte patterns from Ghidra MCP files')
    parser.add_argument('--input', '-i', required=True, help='Input MCP file path')
    
    args = parser.parse_args()
    
    if not os.path.exists(args.input):
        print(f"Error: Input file {args.input} does not exist")
        return
        
    bytes_array = parse_mcp_file(args.input)
    
    if bytes_array:
        print(f"Found {len(bytes_array)} bytes:")
        # Format output in 16-byte groups
        for i in range(0, len(bytes_array), 16):
            group = bytes_array[i:i+16]
            hex_str = ", ".join([f"0x{b:02X}" for b in group])
            print(f"    {hex_str}")
    else:
        print("No pattern data found")

if __name__ == "__main__":
    main()