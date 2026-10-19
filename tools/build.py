#!/usr/bin/env python3
"""Compile and link the complete game without the original executable.

Historical VC5 SP3 and DirectX SDK files are required; game assets and reference
repositories are not build inputs. The output is never executed automatically.
Use tools/verify.py or tools/relink.py separately for oracle-based verification.
"""
import sys
import relink

if __name__ == '__main__':
    raise SystemExit(relink.main(['--standalone', '--out-dir', str(relink.ROOT / 'build/standalone'),
                                 *sys.argv[1:]]))
