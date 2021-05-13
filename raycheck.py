#!/usr/bin/env python3

import re
import sys
import os
import shutil
import subprocess
import argparse
import colorama
from colorama import Fore, Style
from zipfile import ZipFile
import tarfile
from scipy.misc import imread
import numpy as np
import hashlib
import pickle
from math import sqrt

def _msg(text, level, color):
    return Style.BRIGHT+color+level+Fore.RESET+Style.NORMAL+text

def compare(imagefn, reffn, rel, args):
    image = imread(imagefn)
    ref = imread(reffn)
    image = image.reshape((-1, 1))
    ref = ref.reshape((-1, 1))
    '''
    Root-mean-square error
    '''
    delta = image - ref
    rms = np.linalg.norm(delta) / sqrt(delta.size)
    return rms

def check_dirs(dirs):
    # print('checking dirs {}'.format(dirs))
    for d in dirs:
        if not d:
            continue
        if not os.path.exists(d):
            print('{} does not exist'.format(d))
            return False
        if not os.path.isdir(d):
            print('{} is not a directory'.format(d))
            return False
    return True

def check_files(files):
    # print('checking files {}'.format(files))
    for f in files:
        if not f:
            continue
        if not os.path.exists(f):
            print('{} does not exist'.format(f))
            return False
        if not os.path.isfile(f):
            print('{} is not a file'.format(f))
            return False
    return True

BUF_SIZE = 32 * 1024

def hashfile(sha, path):
    with open(path, 'rb') as f:
        while True:
            data = f.read(BUF_SIZE)
            if not data:
                break
            sha.update(data)

def check_ref_signature(refcache_dir, refbin, json, cubemap):
    sha = hashlib.sha256()
    signature_path = os.path.join(refcache_dir, 'signature')
    hashfile(sha, refbin)
    if json:
        hashfile(sha, json)
    if cubemap:
        hashfile(sha, cubemap)
    need_flush = False
    need_update = False
    new_hash = sha.hexdigest().encode()
    if not os.path.exists(signature_path):
        # print("{} not exists, creating".format(signature_path))
        signature = new_hash
        need_update = True
    else:
        with open(signature_path, 'rb') as f:
            signature = f.readline()
        # print("Hexing was {} expect {}".format(signature, new_hash))
        if signature != new_hash:
            # print("{} != {}".format(signature, new_hash))
            signature = new_hash
            need_flush = True
            need_update = True
    if need_flush:
        print(_msg('FLUSING REFCACHE', '[INFO] ', Fore.WHITE))
        shutil.rmtree(refcache_dir)
    os.makedirs(refcache_dir, exist_ok=True)
    if need_update:
        with open(signature_path, 'wb') as f:
            f.write(signature)
    return need_update

def raycheck(args):
    args.refcache = args.out + '/refcache' if not args.refcache else args.refcache
    args.diffout = args.out + '/diff' if not args.diffout else args.diffout
    args.mtgout = args.out + '/mtg' if not args.mtgout else args.mtgout
    # print(args)

    scene_dir = args.scenes
    os.makedirs(args.out, exist_ok=True)
    if not check_dirs([scene_dir, args.out]) or (args.exec and not check_files([args.exec, args.ref])):
        return
    cmdargs = ['-r', '5']
    if args.json:
        cmdargs += ['-j', args.json]
    if args.cubemap:
        cmdargs += ['-c', args.cubemap]
    refcache_dir = args.refcache
    if args.ref:
        check_ref_signature(refcache_dir, args.ref, args.json, args.cubemap)
    report_dict = dict()
    for root, dirs, files in os.walk(scene_dir):
        rel_dir = os.path.relpath(root, start=scene_dir)
        # print('root {} rel {}'.format(root, rel_dir))
        os.makedirs(os.path.join(args.out, 'image', rel_dir), exist_ok=True)
        os.makedirs(os.path.join(args.out, 'stdio', rel_dir), exist_ok=True)
        os.makedirs(os.path.join(refcache_dir, rel_dir), exist_ok=True)
        os.makedirs(os.path.join(args.diffout, rel_dir), exist_ok=True)
        os.makedirs(os.path.join(args.mtgout, rel_dir), exist_ok=True)
        if args.diffout:
            os.makedirs(os.path.join(args.diffout, rel_dir), exist_ok=True)
        for fn in files:
            if not fn.endswith('.ray'):
                continue
            rayfn = os.path.join(root, fn)
            ray_rel_from_scene = os.path.relpath(rayfn, start=scene_dir)
            relbase, _ = os.path.splitext(ray_rel_from_scene)
            # print('processing {}'.format(relbase))
            imagefn = os.path.join(args.out, 'image', relbase + '.png')
            refimagefn = os.path.join(refcache_dir, relbase + '.std.png')
            stdoutfn = os.path.join(args.out, 'stdio', relbase + '.out')
            stderrfn = os.path.join(args.out, 'stdio', relbase + '.err')
            # print('{} -> {} vs {}'.format(rayfn, imagefn, refimagefn))
            #
            if args.ref and not os.path.exists(refimagefn):
                alist = [args.ref] + cmdargs + [rayfn, refimagefn]
                print(alist)
                subprocess.run(alist)
            '''
            Allow only running ray.std
            '''
            if args.exec:
                with open(stdoutfn, 'w') as stdout, open(stderrfn, 'w') as stderr:
                    alist = [args.exec] + cmdargs + [rayfn, imagefn]
                    print(alist)
                    try:
                        subprocess.run(alist, stdout=stdout, stderr=stderr, timeout=args.timelimit)
                    except subprocess.TimeoutExpired:
                        print(_msg(relbase + ' Timeout', '[ERROR] ', Fore.RED))
                    except:
                        print(_msg(relbase + ' (Unknown)', '[ERROR] ', Fore.RED))
            rms = 1000.0
            if os.path.exists(imagefn) and os.path.exists(refimagefn):
                rms = compare(imagefn, refimagefn, relbase, args)
            if args.nocompare:
                print(_msg(relbase + ' Processed', '[INFO] ', Fore.WHITE))
                continue
            '''
            Compare and generate diff images
            '''
            if rms < args.maxrms:
                print(_msg(relbase + ' RMS: {}'.format(rms), '[PASS] ', Fore.GREEN))
            else:
                print(_msg(relbase + ' RMS: {}'.format(rms), '[WARNING] ', Fore.YELLOW))
                if args.diffout:
                    difffn = os.path.join(args.diffout, relbase + '.diff.png')
                    subprocess.run(['compare', imagefn, refimagefn, difffn])
                    if args.mtgout:
                        mtgfn = os.path.join(args.mtgout, relbase + '.mtg.png')
                        subprocess.run(['montage', imagefn, refimagefn, difffn,
                            '-tile', '3x', '-geometry', '+1+1', mtgfn])
            report_dict[relbase] = rms
    if (not args.nocompare) and args.report:
        ''' Only write to report if nocompare is not enabled '''
        with open(args.report, 'wb') as f:
            pickle.dump(report_dict, f)

if __name__ == '__main__':
    colorama.init()
    parser = argparse.ArgumentParser(description='Grading your ray tracer',
            formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--exec', metavar='RAY',
            help='Executable file of your ray tracer',
            default='build/bin/ray')
    parser.add_argument('--ref', metavar='RAY.STD',
            help='Reference ray tracer',
            default='ray-solution')
    parser.add_argument('--scenes', metavar='DIRECTORY',
            help='Directory that stores .ray files.',
            default='assets/scenes')
    parser.add_argument('--out', metavar='DIRECTORY',
            help='Output directory',
            default='raycheck.out')
    '''
    This option allows sharing the refcache
    '''
    parser.add_argument('--refcache', metavar='DIRECTORY',
            help='Reference image cache directory',
            default='')
    parser.add_argument('--diffout', metavar='DIRECTORY',
            help='Diff image directory',
            default='')
    parser.add_argument('--mtgout', metavar='DIRECTORY',
            help='Merged image output directory',
            default='')
    parser.add_argument('--report', metavar='FILE',
            help='Report file',
            default='')
    parser.add_argument('--json', metavar='JSON',
            help='JSON configuration file for testing',
            default='')
    parser.add_argument('--cubemap', metavar='FILE',
            help='One texture file for cubemapping',
            default='')
    parser.add_argument('--timelimit', metavar='SECONDS',
            help='Time limit in seconds, NOTE: the default limit is used during grading',
            type=int,
            default=180)
    parser.add_argument('--maxrms', metavar='NUMBER',
            help='Maximum allowed root-mean-square error',
            type=float,
            default=10.0)
    parser.add_argument('--nocompare',
            help='Only render the image, do not compare',
            action='store_true')
    args = parser.parse_args()
    if args.nocompare and args.report:
        print(_msg('CANNOT SET --nocompare and --report at the same time', '[CRIT] ', Fore.RED))
        exit()
    raycheck(args)
