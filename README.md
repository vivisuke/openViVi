# openViVi
open source version of ViVi text editor, which supports standard text editor features and vi commands.

オープンソース版ViViテキストエディタ、
標準的なテキストエディタの機能とviコマンドをサポートします。

# ■ スケジュール・進捗
- 2020年4月より本格的に開発開始 ~~予定~~
- 現在（５月上旬）は作者がドキュメント編集を行うために必要な機能実装・重要問題対処中
  - ５月中旬頃から基本viコマンド実装予定
  - 当面は機能実装期間で、（作者の主観で）ささいな問題・環境依存問題の対処は行わない
- スポンサーが多いほど開発に時間をかけます
- ViVi 1.x 相当をまず実装予定（期間：1～2年？）→ ViVi 7.0.xxx としてリリース？
  - 作者がドキュメント・ソース編集を行うために必要な機能から順に実装
  - （暫定）実装優先順位：
    1. ~~エディットバッファ（非GUI）~~ done
    1. ~~テキストエディタ基本機能（GUI、[1] ビューワ機能（ファイルオープン、テキスト表示、ミニマップ表示、スクロール）, [2] 基本編集機能（キー・マウスによるテキスト選択、文字入力・削除、undo/redo、検索））~~ done
    1. 作者がドキュメント編集するための必須機能（~~インデント・逆インデント、オートインデント、~~ 罫線モード、罫線保護編集、テキストアライン、MDIタブ切り替え、日付等入力、リナンバ、アウトライン）
    1. 基本 vi コマンド（hjkl-+%wWbBGeEgg0^$fFtT;,/?nN*iaIArRoOsSpPxXdd{c|d|y}mv.uUz）
    1. 基本以外の機能（対応括弧強調・移動、多段MDIタブ、置換ダイアログ、検索オプション、grep、BOX選択、D&D編集、クリップボード履歴、文字列変換、単語補完、禅コーディング、画面分割、折返し表示、折り畳み、上書きモード、終了時ファイル復帰、水平スクロールバー、ルーラー表示、マークダウン、印刷・印刷プレビュー、メニュー・ダイアログ等日本語化、キーボードマクロ、Des暗号化・復号化）
    1. 基本以外の vi コマンド
    1. 文書比較
    1. スクリプト（QScriptEngine 使用？）
    1. マルチカーソル？
    1. マークダウンプレビュー・WYSIWYG編集
    1. HTMLビュー？
    1. ファイルシステム（ドッキングペイン？）？
- ViVi 7.1.xxx
  - バイナリモード
  - CSVモード
  - キーボードカスタマイズ
  - メニューカスタマイズ
  - 巨大ファイル（１GB超）対応？

# ■ 開発環境
- Windows 10, Visual Studio 2019, C++, ~~当面は32~~ 64bitモード only
- GUI は Qt5（VS2019 VS tools） を使用   ~~？または今更 MFC ？~~
  - Qt であれば Mac, Linux でもビルド可能なはず
  
# ■ ビルド方法
- Visual Studio 2019, Qt5.14(64ビットモード必須) をインストール
  - Qt download: https://www.qt.io/download-qt-installer
- VS2019：拡張機能 > 拡張機能の管理 から Qt VS Tools をインストール
- リポジトリの vivi/vivi.sln を VS2019 で開き、ビルド

# ■ 開発管理方針
### プロジェクトへのご意見・ご要望
本プロジェクトへのご意見・ご要望は、スポンサー様からのみ受け付けます。
ただし、寄付金額に比例した回数制限 or 期限制限を設けます（詳細未定）。
また、必ずしもそれらに従うというわけではありません。
それらを参考にし、作者の独断と偏見・その時点での諸々の都合で、作者がプロジェクト管理上の全ての決定を行います（異論は認めない）。

### 問題報告
問題報告はどなたでも行うことができます。
ただし、問題対処優先順位は作者が独断と偏見で決定します。場合によってはNPTF（No Plan To Fix）とします（異論は認めない）。

### 仕様かどうかの質問
明らかに不適切な動作が仕様なのかどうかの質問は、作者が極めて不快になるのでご遠慮ください。

# ■ スポンサー
本プロジェクトは下記スポンサー様のご支援を受けています。ありがとうございます。
- J.N 様 （2020年4月 ￥5,000)
- YS INC 様 （2020年4月 ￥1,000)
- amnesia828 様 （2020年4月 ￥10,000)
- N.W 様 （2020年4月 ￥5,000)
- mkogax 様 （2020年4月 ￥3,000)
- K.Y 様 （2020年3月 ￥5,000)
- つぼい 様 （2020年3月 ￥2,000)
- Y.K 様 （2020年2月 ￥1,000)

スポンサーになって支援してあげようという方は、以下のページから寄付を行ってください。

http://vivi.dyndns.org/vivi/donate.html

~~本プロジェクト管理者（ntsuda@master.email.ne.jp または https://twitter.com/vivisuke ）までご連絡ください。~~

よろしくおながいしますー
