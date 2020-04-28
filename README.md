# openViVi
open source version of ViVi text editor, which supports standard text editor features and vi commands.

オープンソース版ViViテキストエディタ、
標準的なテキストエディタの機能とviコマンドをサポートします。

## スケジュール
- 2020年4月より開発開始 ~~予定~~
- 現在（４月下旬）はテキストエディタ基本機能を実装中
  - ５月中旬頃から基本viコマンド実装予定
- スポンサーが多いほど開発に時間をかけます
- ViVi 1.x 相当をまず実装予定（期間：1～2年？）→ ViVi 7.0.xxx としてリリース？
  - （暫定）優先順位：
    1. エディットバッファ（非GUI）
    1. テキストエディタ基本機能（GUI、[1] ビューワ機能, [2] 基本編集機能）
    1. 基本 vi コマンド
    1. 文字列変換など基本以外の機能（折返し表示、折り畳み、対応括弧強調・移動、BOX選択、grep、D&D編集、日付等入力、単語補完、禅コーディング、画面分割、ルーラー表示、終了時ファイル復帰、水平スクロールバー、多段MDIタブ、マークダウン、印刷・印刷プレビュー、メニュー・ダイアログ等日本語化）
    1. 基本以外の vi コマンド
    1. アウトライン
    1. 罫線モード
    1. 文書比較
    1. スクリプト
- ViVi 7.1.xxx
  - バイナリモード
  - CSVモード

## 開発環境
- Windows 10, Visual Studio 2019, C++, ~~当面は32~~ 64bitモード only
- GUI は Qt5（VS2019 VS tools） を使用   ~~？または今更 MFC ？~~
  - Qt であれば Mac, Linux でもビルド可能なはず
  
## ビルド方法
- Visual Studio 2019, Qt5.14(64ビットモード必須) をインストール
- VS2019：拡張機能 > 拡張機能の管理 から Qt VS Tools をインストール
- リポジトリの vivi/vivi.sln を VS2019 で開き、ビルド

## スポンサー
本プロジェクトへのご意見・ご要望は、スポンサー様からのみ受け付けます。
ただし、寄付金額に比例した回数制限 or 期限制限があり（詳細未定）、必ずしもそれらに従うというわけではありません。
それらを参考にし、作者の独断と偏見・その時点での諸々の都合で、作者がプロジェクト管理上の全ての決定を行います。

※問題報告はどなたでも行うことができます。

本プロジェクトは下記スポンサー様のご支援を受けています。ありがとうございます。
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
