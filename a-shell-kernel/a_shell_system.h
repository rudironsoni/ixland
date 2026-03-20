// a_shell_system.h - Main framework header
// This is the public header for the a-shell-kernel framework

#ifndef A_SHELL_SYSTEM_H
#define A_SHELL_SYSTEM_H

// Only import Foundation/UIKit for Objective-C/C++
#ifdef __OBJC__
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#endif

// Include the main implementation header from a_shell_system directory
// NOTE: This does NOT include the Linux compatibility headers with macros
// Those are in include/a_shell_kernel.h for ported software
#include "a_shell_system/a_shell_system.h"

#endif
