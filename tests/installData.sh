# Download bag of words vocabulary
curl https://github.com/SolarFramework/SolARModuleFBOW/releases/download/fbowVocabulary/fbow_voc.zip -L -o fbow_voc.zip
mkdir -p data/fbow_voc
unzip -o fbow_voc.zip -d ./data/fbow_voc
rm fbow_voc.zip
curl https://repository.solarframework.org/generic/FbowVoc/popsift_uint8.fbow -L -o data/fbow_voc/popsift_uint8.fbow