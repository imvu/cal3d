
from imvu import libzero
from imvu.avatarwindow import ModelTextureData

class materialTestData:

    material_template =\
        "<HEADER MAGIC=\"XRF\" VERSION=\"919\" />"\
        "<MATERIAL NUMMAPS=\"2\">"\
        "<AMBIENT>149 149 149 0</AMBIENT>"\
        "<DIFFUSE>255 255 255 255</DIFFUSE>"\
        "<SPECULAR>229 229 229 0</SPECULAR>"\
        "<SHININESS>0</SHININESS>"\
        "<MAP TYPE=\"Diffuse Color\">%s</MAP>"\
        "<MAP TYPE=\"Opacity\">%s</MAP>"\
        "</MATERIAL>"


    material_no_textures =\
        "<HEADER MAGIC=\"XRF\" VERSION=\"919\" />"\
        "<MATERIAL NUMMAPS=\"0\">"\
        "<AMBIENT>149 149 149 0</AMBIENT>"\
        "<DIFFUSE>255 255 255 255</DIFFUSE>"\
        "<SPECULAR>229 229 229 0</SPECULAR>"\
        "<SHININESS>0</SHININESS>"\
        "</MATERIAL>"

    material_without_opacity_template =\
        "<HEADER MAGIC=\"XRF\" VERSION=\"919\" />"\
        "<MATERIAL NUMMAPS=\"2\">"\
        "<AMBIENT>149 149 149 0</AMBIENT>"\
        "<DIFFUSE>255 255 255 255</DIFFUSE>"\
        "<SPECULAR>229 229 229 0</SPECULAR>"\
        "<SHININESS>0</SHININESS>"\
        "<MAP TYPE=\"Diffuse Color\">%s</MAP>"\
        "</MATERIAL>"

    material_with_unsupported_template =\
        "<HEADER MAGIC=\"XRF\" VERSION=\"919\" />"\
        "<MATERIAL NUMMAPS=\"2\">"\
        "<AMBIENT>149 149 149 0</AMBIENT>"\
        "<DIFFUSE>255 255 255 255</DIFFUSE>"\
        "<SPECULAR>229 229 229 0</SPECULAR>"\
        "<SHININESS>0</SHININESS>"\
        "<MAP TYPE=\"Diffuse Color\">%s</MAP>"\
        "<MAP TYPE=\"Opacity\">%s</MAP>"\
        "<MAP TYPE=\"Unsupported\">%s</MAP>"\
        "</MATERIAL>"

    

    basecoat_material_template =\
        "<HEADER MAGIC=\"XRF\" VERSION=\"919\" />"\
        "<MATERIAL NUMMAPS=\"2\">"\
        "<AMBIENT>149 149 149 0</AMBIENT>"\
        "<DIFFUSE>255 255 255 255</DIFFUSE>"\
        "<SPECULAR>229 229 229 0</SPECULAR>"\
        "<SHININESS>0</SHININESS>"\
        "<MAP TYPE=\"Diffuse Color\">%s</MAP>"\
        "<MAP TYPE=\"Opacity\">%s</MAP>"\
        "</MATERIAL>"

   

    def __init__(self, 
        diffuseMapName="foo_diffuse.png", 
        opacityMapName="foo_opacity.png"):
        self.stringToTextureMap = {}
        self.diffuseMapName = diffuseMapName
        self.opacityMapName = opacityMapName
        self.basecoatDiffuseMapName = "basecoat_diffuse.jpg"
        self.basecoatOpacityMapName = "basecoat_opacity.jpg"        
        self.material = materialTestData.material_template % (self.diffuseMapName, self.opacityMapName)        
        self.material_without_opacity = materialTestData.material_without_opacity_template % self.diffuseMapName
        self.material_with_unsupported = materialTestData.material_with_unsupported_template % (self.diffuseMapName, self.opacityMapName, "unsupported.jpg")
        self.basecoat_material = materialTestData.basecoat_material_template % (self.basecoatDiffuseMapName, self.basecoatOpacityMapName) 

    def setUp(self):  
        self.stringToTextureMap = {}   

    def defineTexture(self, name, modelTextureData):
        self.stringToTextureMap[name] = modelTextureData

    def defineTextureColor(self, name, r, g, b, size=libzero.Vec2i(1, 1)):
        rgb = chr(r) + chr(g) + chr(b)
        s = size.x * size.y * rgb
        self.stringToTextureMap[name] = ModelTextureData(name, s, size.x, size.y, size.x, size.y, False)

    def getTextureData(self, name):
        return self.stringToTextureMap[name]

meshDataPrefix =\
    "<HEADER MAGIC=\"XMF\" VERSION=\"919\" />"\
    "<MESH NUMSUBMESH=\"1\">"

submeshTemplate =\
    "   <SUBMESH NUMVERTICES=\"3\" NUMFACES=\"1\" NUMLODSTEPS=\"0\" NUMSPRINGS=\"0\" NUMMORPHS=\"1\" NUMTEXCOORDS=\"1\" MATERIAL=\"%d\">"\
    "       <VERTEX NUMINFLUENCES=\"1\" ID=\"0\">"\
    "           <POS>5759.05 -1176.88 -0.00023478</POS>"\
    "           <NORM>1.27676e-008 2.40249e-008 -1</NORM>"\
    "           <COLOR>0 0 0</COLOR>"\
    "           <TEXCOORD>0.99311 0.00973237</TEXCOORD>"\
    "           <INFLUENCE ID=\"0\">1</INFLUENCE>"\
    "       </VERTEX>"\
    "       <VERTEX NUMINFLUENCES=\"2\" ID=\"1\">"\
    "           <POS>-5759.05 -1176.88 -0.000413365</POS>"\
    "           <NORM>1.55047e-008 -2.86491e-008 -1</NORM>"\
    "           <COLOR>0 0 0</COLOR>"\
    "           <TEXCOORD>0.99311 0.982444</TEXCOORD>"\
    "           <INFLUENCE ID=\"0\">0.5</INFLUENCE>"\
    "           <INFLUENCE ID=\"1\">0.5</INFLUENCE>"\
    "       </VERTEX>"\
    "       <VERTEX NUMINFLUENCES=\"1\" ID=\"2\">"\
    "           <POS>-5759.05 -3274.86 -0.000507484</POS>"\
    "           <NORM>1.11221e-008 6.89228e-008 -1</NORM>"\
    "           <COLOR>0 0 0</COLOR>"\
    "           <TEXCOORD>0.79062 0.982444</TEXCOORD>"\
    "           <INFLUENCE ID=\"1\">1</INFLUENCE>"\
    "       </VERTEX>"\
    "       <MORPH NAME=\"ogle\" NUMBLENDVERTS=\"1\" MORPHID=\"0\">"\
    "           <BLENDVERTEX VERTEXID=\"2\">"\
    "               <POSITION>6000 -1500 0</POSITION>"\
    "               <NORMAL>0 0.707 -0.707</NORMAL>"\
    "               <TEXCOORD>0.0 0.25</TEXCOORD>"\
    "           </BLENDVERTEX>"\
    "       </MORPH>"\
    "       <FACE VERTEXID=\"0 1 2\" />"\
    "    </SUBMESH>"

meshDataSuffix =\
    "</MESH>"

def makeMeshTestData(materialIds):
    meshData = meshDataPrefix
    for materialId in materialIds:
        meshData += submeshTemplate % (materialId,)
    meshData += meshDataSuffix
    return meshData


skeletonTestData = \
"""<HEADER VERSION=\"910\" MAGIC="XSF" />
    <SKELETON SCENEAMBIENTCOLOR="1 1 1" NUMBONES="2">
    <BONE NAME="FooRoot" NUMCHILDS="1" ID="0">
        <TRANSLATION>0 0 -30</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 0 30</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>-1</PARENTID>
        <CHILDID>1</CHILDID>
    </BONE>
    <BONE NAME="FooChild" NUMCHILDS="0" ID="1">
        <TRANSLATION>0 -10 500</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 10 -470</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>0</PARENTID>
    </BONE>
</SKELETON>"""


animationTestData = \
"""
<HEADER VERSION="910" MAGIC="XAF" />
<ANIMATION NUMTRACKS="2" DURATION="2">
    <TRACK NUMKEYFRAMES="2" BONEID="0">
        <KEYFRAME TIME="0">
            <TRANSLATION>0 10 20</TRANSLATION>
            <ROTATION>0.569964 -0.437345 0.517176 -0.465187</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="1">
            <TRANSLATION>0 10 20</TRANSLATION>
            <ROTATION>0.564503 -0.461901 0.501256 -0.465538</ROTATION>
        </KEYFRAME>
    </TRACK>
    <TRACK NUMKEYFRAMES="2" BONEID="1">
        <KEYFRAME TIME="0">
            <TRANSLATION>40 50 60</TRANSLATION>
            <ROTATION>0.322539 0.588375 -0.579296 0.462817</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="1">
            <TRANSLATION>40 50 60</TRANSLATION>
            <ROTATION>-0.0611443 -0.627387 0.771113 0.0896201</ROTATION>
        </KEYFRAME>
    </TRACK>
</ANIMATION>
"""


attachmentSkeletonTestData = \
"""<HEADER VERSION=\"910\" MAGIC="XSF" />
    <SKELETON SCENEAMBIENTCOLOR="1 1 1" NUMBONES="2">
    <BONE NAME="AuxRoot" NUMCHILDS="1" ID="0">
        <TRANSLATION>0 0 -30</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 0 30</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>-1</PARENTID>
        <CHILDID>1</CHILDID>
    </BONE>
    <BONE NAME="AuxChild" NUMCHILDS="0" ID="1">
        <TRANSLATION>0 -10 500</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 10 -470</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>0</PARENTID>
    </BONE>
</SKELETON>"""
