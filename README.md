# プロジェクト概要
本プロジェクトは、GASPALS Projectをベースに構築した、一人称視点（FPS）のシューティングゲーム開発リポジトリです。
Gameplay Ability System (GAS) を核とした拡張性の高いシステム設計と、StateTreeを用いた高度な敵AIの実装に注力しています。
※リポジトリ軽量化のため、現在は**Source（C++コード）とConfig（プロジェクト設定）**を中心に公開しています。

---

# 主な実装システム
本プロジェクトでは、以下のシステムをC++およびGASを用いて構築しました。

## キャラクタームーブメント & アクション
- スライディングシステムの実装
- GASを用いた武器の射撃・リロード・切り替えシステム
- GameplayTagによるキャラクター状態の厳密な管理

## AIシステム (StateTree & EQS)
- StateTreeを利用した敵AIの意思決定ロジック
- Environment Query System (EQS) による索敵・ポジショニング

## ゲームフレームワーク
- HealthComponentを活用したダメージ処理と体力管理
- ProceduralChunkを用いたマップの自動生成
- GameProgressSubsystemによるゲーム進行イベントの制御

# 開発履歴とリポジトリ管理について
過去の実装および最適化の内容です。これら全てのロジックは現在のソースコードに統合されています。

## 戦闘システム
- ADS（エイム）機能、弾種追加、銃声・ヒット音アセットの統合
- リロードアニメーションの実装

## システム基盤
- AssetManager / GameInstance の独自拡張
- LevelConfigDataAsset によるデータ駆動設計
- 不要なバイナリの除外による軽量化

## AI・環境
- EnemyStats / CombatLibrary の構築
- ChunkManager によるマップ生成の最適化
