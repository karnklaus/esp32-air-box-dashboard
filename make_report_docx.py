from __future__ import annotations

import datetime as dt
import html
import re
import struct
import zipfile
from pathlib import Path


ROOT = Path(__file__).resolve().parent
SOURCE_MD = ROOT / "รายงานโครงงาน_Air_Box.md"
OUTPUT_DOCX = ROOT / "รายงานโครงงาน_Air_Box.docx"

EMU_PER_INCH = 914400
MAX_IMAGE_WIDTH_IN = 6.0
MAX_IMAGE_HEIGHT_IN = 8.0


def xml_escape(text: str) -> str:
    return html.escape(text, quote=False)


def png_size(path: Path) -> tuple[int, int]:
    with path.open("rb") as f:
        sig = f.read(8)
        if sig != b"\x89PNG\r\n\x1a\n":
            raise ValueError(f"Unsupported image type for {path.name}; only PNG is supported")
        length = struct.unpack(">I", f.read(4))[0]
        chunk_type = f.read(4)
        if chunk_type != b"IHDR" or length < 8:
            raise ValueError(f"Invalid PNG header for {path.name}")
        width, height = struct.unpack(">II", f.read(8))
        return width, height


def fit_image_emu(width_px: int, height_px: int) -> tuple[int, int]:
    dpi = 96.0
    width_in = width_px / dpi
    height_in = height_px / dpi
    scale = min(MAX_IMAGE_WIDTH_IN / width_in, MAX_IMAGE_HEIGHT_IN / height_in, 1.0)
    return int(width_in * scale * EMU_PER_INCH), int(height_in * scale * EMU_PER_INCH)


def paragraph_xml(text: str, *, bold: bool = False, size_half_points: int = 24, style: str | None = None) -> str:
    style_xml = f"<w:pStyle w:val=\"{style}\"/>" if style else ""
    run_props = [
        "<w:rFonts w:ascii=\"TH Sarabun New\" w:hAnsi=\"TH Sarabun New\" w:eastAsia=\"TH Sarabun New\"/>",
        f"<w:sz w:val=\"{size_half_points}\"/>",
        f"<w:szCs w:val=\"{size_half_points}\"/>",
    ]
    if bold:
        run_props.append("<w:b/>")
    return (
        "<w:p>"
        f"<w:pPr>{style_xml}</w:pPr>"
        "<w:r>"
        f"<w:rPr>{''.join(run_props)}</w:rPr>"
        f"<w:t xml:space=\"preserve\">{xml_escape(text)}</w:t>"
        "</w:r>"
        "</w:p>"
    )


def image_paragraph_xml(rel_id: str, name: str, cx: int, cy: int, docpr_id: int) -> str:
    return f"""
<w:p>
  <w:r>
    <w:drawing>
      <wp:inline distT="0" distB="0" distL="0" distR="0"
        xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing">
        <wp:extent cx="{cx}" cy="{cy}"/>
        <wp:docPr id="{docpr_id}" name="{xml_escape(name)}"/>
        <a:graphic xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">
          <a:graphicData uri="http://schemas.openxmlformats.org/drawingml/2006/picture">
            <pic:pic xmlns:pic="http://schemas.openxmlformats.org/drawingml/2006/picture">
              <pic:nvPicPr>
                <pic:cNvPr id="{docpr_id}" name="{xml_escape(name)}"/>
                <pic:cNvPicPr/>
              </pic:nvPicPr>
              <pic:blipFill>
                <a:blip r:embed="{rel_id}" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"/>
                <a:stretch><a:fillRect/></a:stretch>
              </pic:blipFill>
              <pic:spPr>
                <a:xfrm>
                  <a:off x="0" y="0"/>
                  <a:ext cx="{cx}" cy="{cy}"/>
                </a:xfrm>
                <a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
              </pic:spPr>
            </pic:pic>
          </a:graphicData>
        </a:graphic>
      </wp:inline>
    </w:drawing>
  </w:r>
</w:p>
""".strip()


def build_document():
    lines = SOURCE_MD.read_text(encoding="utf-8").splitlines()
    body_parts: list[str] = []
    image_rels: list[tuple[str, Path]] = []
    rel_index = 1
    docpr_id = 1

    for raw_line in lines:
        line = raw_line.rstrip()
        if not line:
            body_parts.append(paragraph_xml(""))
            continue

        image_match = re.match(r"!\[(.*?)\]\((.*?)\)", line)
        if image_match:
            rel_id = f"rId{rel_index}"
            rel_index += 1
            image_path = (ROOT / image_match.group(2)).resolve()
            image_rels.append((rel_id, image_path))
            w_px, h_px = png_size(image_path)
            cx, cy = fit_image_emu(w_px, h_px)
            body_parts.append(image_paragraph_xml(rel_id, image_path.name, cx, cy, docpr_id))
            docpr_id += 1
            continue

        if line.startswith("### "):
            body_parts.append(paragraph_xml(line[4:], bold=True, size_half_points=28))
            continue
        if line.startswith("## "):
            body_parts.append(paragraph_xml(line[3:], bold=True, size_half_points=34))
            continue
        if line.startswith("# "):
            body_parts.append(paragraph_xml(line[2:], bold=True, size_half_points=40))
            continue
        if re.match(r"\d+\.\s", line):
            body_parts.append(paragraph_xml(line, size_half_points=24))
            continue
        if line.startswith("- "):
            body_parts.append(paragraph_xml(f"• {line[2:]}", size_half_points=24))
            continue

        body_parts.append(paragraph_xml(line, size_half_points=24))

    sect_pr = """
<w:sectPr>
  <w:pgSz w:w="11906" w:h="16838"/>
  <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="708" w:footer="708" w:gutter="0"/>
</w:sectPr>
""".strip()

    document_xml = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:wpc="http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas"
 xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006"
 xmlns:o="urn:schemas-microsoft-com:office:office"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
 xmlns:m="http://schemas.openxmlformats.org/officeDocument/2006/math"
 xmlns:v="urn:schemas-microsoft-com:vml"
 xmlns:wp14="http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing"
 xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
 xmlns:w10="urn:schemas-microsoft-com:office:word"
 xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
 xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml"
 xmlns:wpg="http://schemas.microsoft.com/office/word/2010/wordprocessingGroup"
 xmlns:wpi="http://schemas.microsoft.com/office/word/2010/wordprocessingInk"
 xmlns:wne="http://schemas.microsoft.com/office/2006/wordml"
 xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape"
 mc:Ignorable="w14 wp14">
  <w:body>
    {''.join(body_parts)}
    {sect_pr}
  </w:body>
</w:document>
"""

    rels = [
        """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rIdStyles" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
</Relationships>
"""
    ]

    rel_entries = [
        '<Relationship Id="rIdStyles" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>'
    ]
    for rel_id, image_path in image_rels:
        rel_entries.append(
            f'<Relationship Id="{rel_id}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" Target="media/{image_path.name}"/>'
        )
    document_rels_xml = (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        + "".join(rel_entries)
        + "</Relationships>"
    )

    content_types = [
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>',
        '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">',
        '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>',
        '<Default Extension="xml" ContentType="application/xml"/>',
        '<Default Extension="png" ContentType="image/png"/>',
        '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>',
        '<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>',
        '<Override PartName="/docProps/core.xml" ContentType="application/vnd.openxmlformats-package.core-properties+xml"/>',
        '<Override PartName="/docProps/app.xml" ContentType="application/vnd.openxmlformats-officedocument.extended-properties+xml"/>',
        '</Types>',
    ]

    root_rels = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
  <Relationship Id="rId2" Type="http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties" Target="docProps/core.xml"/>
  <Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties" Target="docProps/app.xml"/>
</Relationships>
"""

    styles_xml = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
  <w:docDefaults>
    <w:rPrDefault>
      <w:rPr>
        <w:rFonts w:ascii="TH Sarabun New" w:hAnsi="TH Sarabun New" w:eastAsia="TH Sarabun New"/>
        <w:sz w:val="24"/>
        <w:szCs w:val="24"/>
      </w:rPr>
    </w:rPrDefault>
  </w:docDefaults>
</w:styles>
"""

    now = dt.datetime.now(dt.UTC).replace(microsecond=0).isoformat().replace("+00:00", "Z")
    core_xml = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties"
 xmlns:dc="http://purl.org/dc/elements/1.1/"
 xmlns:dcterms="http://purl.org/dc/terms/"
 xmlns:dcmitype="http://purl.org/dc/dcmitype/"
 xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <dc:title>รายงานโครงงาน Air Box</dc:title>
  <dc:creator>Codex</dc:creator>
  <cp:lastModifiedBy>Codex</cp:lastModifiedBy>
  <dcterms:created xsi:type="dcterms:W3CDTF">{now}</dcterms:created>
  <dcterms:modified xsi:type="dcterms:W3CDTF">{now}</dcterms:modified>
</cp:coreProperties>
"""

    app_xml = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Properties xmlns="http://schemas.openxmlformats.org/officeDocument/2006/extended-properties"
 xmlns:vt="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes">
  <Application>Microsoft Office Word</Application>
</Properties>
"""

    with zipfile.ZipFile(OUTPUT_DOCX, "w", zipfile.ZIP_DEFLATED) as zf:
        zf.writestr("[Content_Types].xml", "".join(content_types))
        zf.writestr("_rels/.rels", root_rels)
        zf.writestr("docProps/core.xml", core_xml)
        zf.writestr("docProps/app.xml", app_xml)
        zf.writestr("word/document.xml", document_xml)
        zf.writestr("word/styles.xml", styles_xml)
        zf.writestr("word/_rels/document.xml.rels", document_rels_xml)
        for _, image_path in image_rels:
            zf.write(image_path, f"word/media/{image_path.name}")


if __name__ == "__main__":
    build_document()
    print("DOCX_CREATED")
