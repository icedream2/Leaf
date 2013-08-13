package com.tes;

import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.URL;
import java.net.URLConnection;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

import org.htmlparser.Node;
import org.htmlparser.Parser;
import org.htmlparser.filters.AndFilter;
import org.htmlparser.filters.HasAttributeFilter;
import org.htmlparser.filters.TagNameFilter;
import org.htmlparser.tags.Span;
import org.htmlparser.util.NodeList;

public class Generate {
	private static Connection ct=null;
	public static void main(String[] args)  {

		try {
			Class.forName("com.mysql.jdbc.Driver");
			ct=DriverManager.getConnection("jdbc:mysql://localhost:3306/leafdata","root","6356763");
			System.out.println("generate table leaf...");
			generateDatabase(1,500);
			System.out.println("generate table leafC...");
			generateLeafC();
			System.out.println("generate text...");
			writeFile();
		}
		catch (Exception e) {
			// TODO: handle exception
		}
	}
	/*
	 * 根据leafList中的id去下载所有同名的图片
	 */
	
	public static void generateDatabase(int begin,int end){
		PreparedStatement ps=null;
		PreparedStatement ps1=null;
		try {
			ps=ct.prepareStatement("insert into leaf values(?,?)");
			for(int i=begin;i<=end;i++){
				String path="http://www.plantphoto.cn/tu/"+i;
				URL url=new URL(path);		
				URLConnection conn=url.openConnection();
				conn.setDoOutput(true);
				InputStream inputStream=conn.getInputStream();
				InputStreamReader isr=new InputStreamReader(inputStream,"utf-8");
				StringBuffer sb=new StringBuffer();
				BufferedReader in=new BufferedReader(isr);
				String inputLine;
				while((inputLine=in.readLine())!=null){
					sb.append(inputLine);
					sb.append("\n");	
				}
				String result=sb.toString();
				ps.setInt(1, i);	
				String leafName=readLeafName(result);
				ps.setString(2, leafName);
				ps.executeUpdate();
			}
			
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}finally{
			try {
				if(ps1!=null)
					ps1.close();
				if(ps!=null)
					ps.close();
			} catch (SQLException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
		}
	}
	public static void generateLeafC(){
		try{
			PreparedStatement ps=null;
			Statement stm=ct.createStatement();
			ResultSet res = stm.executeQuery("select * from leaflist");
			ps=ct.prepareStatement("insert into leafC values(?,?,?)");
			int beginNumber=1;
			while(res.next()){
				String leafName=res.getString("name");
				int picNum=res.getInt("picNum");
				ps.setInt(1, beginNumber);
				ps.setString(2, leafName);
				ps.setInt(3, picNum);
				ps.executeUpdate();
				beginNumber++;
			}
		}catch(Exception e){
			e.printStackTrace();
		}

	}

	public static String readLeafName(String result)throws Exception{
		Parser parser = Parser.createParser(result, "utf-8"); 
        AndFilter filter = 
          new AndFilter( 
                        new TagNameFilter("span"), 
                       new HasAttributeFilter("id","Label15") 
        ); 
        NodeList n = parser.parse(filter); 
        Node[] nodes=n.toNodeArray();
		String line="";
		Node node=nodes[0];
		Span s=(Span)node;
		line=s.getChildrenHTML();
		line=getName(line);
		return line;
	}
	public static String getName(String child){
		String str="";
		for(int i=0;i<child.length();i++){
			//System.out.println(child.charAt(i));
			if(child.charAt(i)=='>'|| child.charAt(i)=='<' ||child.charAt(i)=='b')
				continue;
			if(child.charAt(i)=='/' && child.charAt(i+1)=='b')
				break;
			str+=child.charAt(i);
		}
		return str;
	}
	
	public static void writeFile()throws Exception{
		FileOutputStream out=new FileOutputStream("D:/test.txt",false);
		PrintStream p=new PrintStream(out);
		Statement stm=ct.createStatement();
		ResultSet res = stm.executeQuery("select * from leafC");
		while(res.next()){
			String name=res.getString("name");
			int id=res.getInt("Id");
			int num=res.getInt("picNum");
			p.print(id);
			p.print("\t"+num);
			p.println("\t"+name);
			
		}
	}
}