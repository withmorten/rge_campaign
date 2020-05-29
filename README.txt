# rge_campaign

Small, relatively platform independent commandline tool to pack and unpack the genie engine's cpn/cpx Campaign files. Should be compatible with both Age of Empires I and II, as the format is so simple that it didn't undergo any changes.

Update: Now has support for extracting and packing DE1 (.aoecpn) and DE2 (.aoe2campaign) files!

No external files or libraries beyond gcc via MINGW in MSYS2, or just a normal linux, are required to compile or run this tool.

## usage

The usage is simple:

    rge_campaign l campaign.cpn

lists the contents of `campaign.cpn`.

    rge_campaign x campaign.cpn outdir

extracts `campaign.cpn` into `outdir`.

    rge_campaign c campaign.cpn campaign scenario1.scn scenario2.scn

packs `scenario1.scn` and `scenario2.scn` in that order into a campaign file `campaign.cpn` with an internal name `campaign`. You can specificy more scenarios than 2. No idea what the limit is.

The extension determines the campaign version to be created:

.cpn/.cpx: AoE1 to AoC
.aoecpn: AoE1 DE
.aoe2campaign: AoE2 DE

Please create an issue if you encounter problems with this tool. I've tested it to the best of my knowledge.

## download

Windows users can download the latest version from the releases page:

https://github.com/withmorten/rge_campaign/releases

The Windows version was compiled with:

    gcc -s -static -O3 -o rge_campaign.exe main.c util.c

Linux users can just compile it themselves with:

    gcc -s -static -O3 -o rge_campaign main.c util.c

Please let me know if there are any problems doing this.
