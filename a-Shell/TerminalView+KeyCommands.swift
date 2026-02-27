//
//  TerminalView+KeyCommands.swift
//  a-Shell
//
//  Created by Nicolas Holzschuch on 27/02/2026.
//  Copyright © 2026 AsheKube. All rights reserved.
//

import Foundation
import UIKit
import SwiftTerm
import ios_system

extension TerminalView {
    
    @objc private func newWindow(_ sender: UIBarButtonItem) {
        if (UIDevice.current.model.hasPrefix("iPad")) {
            UIApplication.shared.requestSceneSessionActivation(nil, userActivity: nil, options: nil)
        }
    }

    @objc private func closeWindow(_ sender: UIBarButtonItem) {
        for scene in UIApplication.shared.connectedScenes {
            if let delegate = scene.delegate as? SceneDelegate {
                if delegate.terminalView == self {
                    delegate.closeWindow()
                    return
                }
            }
        }
    }

    @objc private func increaseTextSize(_ sender: UIBarButtonItem) {
        NSLog("Increase event received")
        for scene in UIApplication.shared.connectedScenes {
            if let delegate = scene.delegate as? SceneDelegate {
                if delegate.terminalView == self {
                    let fontSize = delegate.terminalFontSize ?? factoryFontSize
                    delegate.configWindow(fontSize: fontSize + 1, fontName: nil, backgroundColor: nil, foregroundColor: nil, cursorColor: nil, cursorShape: nil, fontLigature: nil)
                    return
                }
            }
        }
    }

    @objc private func decreaseTextSize(_ sender: UIBarButtonItem) {
        NSLog("Decrease event received")
        for scene in UIApplication.shared.connectedScenes {
            if let delegate = scene.delegate as? SceneDelegate {
                if delegate.terminalView == self {
                    let fontSize = delegate.terminalFontSize ?? factoryFontSize
                    delegate.configWindow(fontSize: fontSize - 1, fontName: nil, backgroundColor: nil, foregroundColor: nil, cursorColor: nil, cursorShape: nil, fontLigature: nil)
                    return
                }
            }
        }
    }


    override open var keyCommands: [UIKeyCommand]? {
        // In case we need keyboard personalization for specific languages
        // var language = textInputMode?.primaryLanguage ?? "en-US"
        var basicKeyCommands = [
            // "discoverabilityTitle:)' was deprecated in iOS 13.0" but it's quite convenient
            UIKeyCommand(input: "n", modifierFlags:.command, action: #selector(newWindow), discoverabilityTitle: "New window"),
            UIKeyCommand(input: "w", modifierFlags:.command, action: #selector(closeWindow), discoverabilityTitle: "Close window"),
            UIKeyCommand(input: "+", modifierFlags:.command, action: #selector(increaseTextSize), discoverabilityTitle: "Bigger text"),
            UIKeyCommand(input: "-", modifierFlags:.command, action: #selector(decreaseTextSize), discoverabilityTitle: "Smaller text"),
            // back/forward one page keys for internal browser:
        ]
        /* Caps Lock remapped to escape: */
        return basicKeyCommands
    }    
}
