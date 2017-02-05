

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Scanner;

/**
 * Created by Arash Afshar.
 *
 * Converts output of frigate to input of scapi
 */
public class FrigateConvert {
    // scapi intput format:
//    from cryptobiu/scapi/blob/master/src/java/edu/biu/scapi/circuits/circuit/BooleanCircuit.java :
//    * The File first lists the number of {@code Gate}s, then the number of parties. <p>
//    * Then for each party: party number, the number of inputs for that party, and following there is a list of indices of each of these input {@code Wire}s.<p>
//    * Next it lists the number of output {@code Wire}s followed by the index of each of these {@code Wires}. <p>
//    * Then for each gate, we have the following: number of inputWires, number of OutputWires inputWireIndices OutputWireIndices and the gate's truth Table (as a 0-1 string).<P>
//            * example file: 1 2 1 1 1 2 1 2 1 3 2 1 1 2 3 0001<p>

    public static void main(String[] args) throws IOException {
        String prefix = args[0] + "/";//"src/main/resources/circuits/";
        String name = args[1]; //"execute";
        String frigate_out_path = prefix + name + ".out";
        String file_path = prefix + name + ".circ";
        String format = args[2];
//        String frigate_out_path = args[0];
//        String file_path = args[1];
        new FrigateConvert(frigate_out_path, file_path, format);
    }

    public static String num_formatter(int num, int max_len){
        String str_rep = Integer.toBinaryString(num);
        int padding = max_len - str_rep.length();
        assert padding >= 0;
        String result;
        if (padding == 0){
            result = String.format("%s", str_rep);
        }else {
            result = String.format("%0" + padding + "d%s", 0, str_rep);
        }
        return result;
    }
    String zeroOut = "";

    public FrigateConvert(String frigate_out_path, String file_path, String format) throws IOException {
        ArrayList<String> p1_inputs = new ArrayList<>();
        ArrayList<String> p2_inputs = new ArrayList<>();
        ArrayList<String> outputs = new ArrayList<>();
        ArrayList<Gate> gates = new ArrayList<>();
        int number_of_gates;
        int max_wire = 0;

        String copy_6 = "0110";


        Scanner scanner = new Scanner(new File(frigate_out_path));
        while (scanner.hasNext()) {
            String line = scanner.nextLine();
            if (line.contains("IN")){
                String party = line.split(" ")[2];
                String wire = line.split(" ")[1];
                if (party.equals("1")) {
                    p1_inputs.add(wire);
                }else {
                    p2_inputs.add(wire);
                }
            }else if (line.contains("OUT")) {
                // we assume evaluator always has the inputs
                String party = line.split(" ")[2];
                String wire = line.split(" ")[1];
                outputs.add(wire);
            }else if (line.contains("copy(6)")){
                String truth = copy_6;
                String tokens[] = line.split(" ");
                Gate gate = new Gate(new String []{tokens[2], tokens[3]}, tokens[1], truth);
                gates.add(gate);
                if(gate.max > max_wire)
                    max_wire = gate.max;
            }else if (line.contains("copy")){
                System.err.println("Not handled");
            }else{
                String tokens[] = line.split(" ");
                int gtype = Integer.parseInt(tokens[0]);
                if(gtype == 0){
                    gtype = 6;
                }
                Gate gate = new Gate(new String []{tokens[2], tokens[3]}, tokens[1], gtype);
                gates.add(gate);
                if(gate.max > max_wire)
                    max_wire = gate.max;
            }

        }

        if(format.equals("nigel")){
            gates.add(0, new Gate(new String[]{"0", "0"}, String.valueOf(max_wire + 1), "0110"));
            zeroOut = String.valueOf(max_wire + 1);
            max_wire++;
            for(int i = 0; i < gates.size(); i++){
                Gate gate = gates.get(i);
                if ( ! (gate.type.equals("0110") || gate.type.equals("1000") || gate.type.equals("0101")))
                    max_wire = normalizeGate(gates, gate, i, max_wire);
            }
        }

        int p1_input_count = p1_inputs.size();
        int p2_input_count = p2_inputs.size();
        int output_count = outputs.size();
        number_of_gates = gates.size();

        //----------------- write to file
        FileOutputStream out = new FileOutputStream(file_path);
        if(format.equals("scapi")){
            out.write((String.valueOf(number_of_gates) + "\n\n").getBytes());
            int number_of_parties = 2;
            out.write((String.valueOf(number_of_parties) + "\n\n").getBytes());
            // p1
            out.write(("1 " + String.valueOf(p1_input_count) + "\n").getBytes());
            for (String inp_wire : p1_inputs) {
                out.write((inp_wire + "\n").getBytes());
            }
            out.write(("\n").getBytes());
            // p2
            out.write(("2 " + String.valueOf(p2_input_count) + "\n").getBytes());
            for (String inp_wire : p2_inputs) {
                out.write((inp_wire + "\n").getBytes());
            }
            out.write(("\n").getBytes());
            // output
            out.write((String.valueOf(output_count) + "\n").getBytes());
            for (String out_wire : outputs) {
                out.write((out_wire + "\n").getBytes());
            }
            out.write(("\n").getBytes());
        } else if(format.equals("nigel")){
            out.write((String.valueOf(number_of_gates) + " " + String.valueOf(max_wire + 1) + "\n").getBytes());
            out.write((String.valueOf(p1_input_count) + " " + String.valueOf(p2_input_count) + " " + String.valueOf(output_count) + "\n\n").getBytes());
        }
        // gates
        for(Gate gate : gates){
            out.write((gate.convertToString(format) + "\n").getBytes());
        }

        out.close();
    }

    int normalizeGate(ArrayList<Gate> gates, Gate gate, int pos, int max_wire){
        if(gate.type.equals("1111")){ // ONE
            gate.type = "0101";
            gate.inputs[0] = zeroOut;
            gate.inputs[1] = "0";
        } else if(gate.type.equals("0010")){ // NOTB_AND
            String A = gate.inputs[0];
            String B = gate.inputs[1];
            String out = gate.output;
            Gate NOT_B = new Gate(new String[]{B, "0"}, String.valueOf(++max_wire), "0101");
            gates.add(pos, NOT_B);

            // make and gate
            gate.type = "1000";
            gate.inputs[1] = String.valueOf(max_wire);
        } else if(gate.type.equals("0001")){ // NOR
            String A = gate.inputs[0];
            String B = gate.inputs[1];
            String out = gate.output;
            Gate NOT_B = new Gate(new String[]{B, "0"}, String.valueOf(++max_wire), "0101");
            gates.add(pos, NOT_B);

            // make XOR gate
            gate.type = "0110";
            gate.inputs[1] = String.valueOf(max_wire);
        } else if(gate.type.equals("0100")){ // NOTA_AND
            String A = gate.inputs[0];
            String B = gate.inputs[1];
            String out = gate.output;
            Gate NOT_A = new Gate(new String[]{A, "0"}, String.valueOf(++max_wire), "0101");
            gates.add(pos, NOT_A);

            // make and gate
            gate.type = "1000";
            gate.inputs[0] = String.valueOf(max_wire);
        } else if(gate.type.equals("1001")){ // NXOR
            // make XOR gate
            gate.type = "0110";

            String out = gate.output;
            Gate NXOR = new Gate(new String[]{out, "0"}, String.valueOf(++max_wire), "0101");
            gates.add(pos + 1, NXOR);

        }
        return max_wire;
    }

    class Gate{
        public String[] inputs;
        public String output;
        public String type;
        public int max;
        public Gate(final String[] inputs, String output, String type){
            this.inputs = inputs;
            this.output = output;
            this.type = type;
            max=0;
            if(Integer.valueOf(inputs[0]) > max){
                max = Integer.valueOf(inputs[0]);
            }
            if(Integer.valueOf(inputs[1]) > max){
                max = Integer.valueOf(inputs[1]);
            }
            if(Integer.valueOf(output) > max){
                max = Integer.valueOf(output);
            }
        }
        public Gate(final String[] inputs, String output, int type){
            this.inputs = inputs;
            this.output = output;
            this.type = truth_table(type);
        }
        private String truth_table(int truth_value){
            String tmp = num_formatter(truth_value, 4);
            return new StringBuilder(tmp).reverse().toString();
        }

        public String convertToString(String format) {
            String rep = "";

            String inp_out_count = String.valueOf(inputs.length) + " 1 ";
            if(format.equals("nigel") && type.equals("0101")){
                rep = "1 1 " + inputs[0] + " ";
            } else{
                rep = inp_out_count;
                for (String inp : inputs){
                    rep += inp + " ";
                }
            }

            rep += output + " ";
            if(format.equals("scapi")){
                rep += type;
            } else if(format.equals("nigel")){
                String type_name = "";
                if(type.equals("0000")){
                    type_name = "ZERO";
                } else if(type.equals("0001")){
                    type_name = "NOR";
                } else if(type.equals("0010")){
                    type_name = "NOTB_AND";
                } else if(type.equals("0011")){
                    type_name = "NOTB";
                } else if(type.equals("0100")){
                    type_name = "NOTA_AND";
                } else if(type.equals("0101")){
                    type_name = "NOTA";
                    type_name = "INV";   // This is not a mistake!!
                } else if(type.equals("0110")){
                    type_name = "XOR";
                } else if(type.equals("0111")){
                    type_name = "NAND";
                } else if(type.equals("1000")){
                    type_name = "AND";
                } else if(type.equals("1001")){
                    type_name = "NXOR";
                } else if(type.equals("1010")){
                    type_name = "A";
                } else if(type.equals("1011")){
                    type_name = "NB_OR";
                } else if(type.equals("1100")){
                    type_name = "B";
                } else if(type.equals("1101")){
                    type_name = "NA_OR";
                } else if(type.equals("1110")){
                    type_name = "OR";
                } else if(type.equals("1111")){
                    type_name = "ONE";
                }
                rep += type_name;
            }
            return rep;
        }
    }
}
