import esphome.codegen as cg

CODEOWNERS = ['@jesserockz']

nfc_ns = cg.esphome_ns.namespace('nfc_old')

NfcTag = nfc_ns.class_('NfcTag')
