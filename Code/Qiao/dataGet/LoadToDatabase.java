package com.test;


import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

import java.io.OutputStream;


import java.net.URL;

import java.sql.*;

import org.htmlparser.Node;

import org.htmlparser.Parser;
import org.htmlparser.filters.AndFilter;
import org.htmlparser.filters.HasAttributeFilter;

import org.htmlparser.filters.TagNameFilter;
import org.htmlparser.tags.Span;
import org.htmlparser.util.NodeList;

public class LoadToDatabase {
	private static Connection ct=null;
	public static void main(String[] args)  {
		if(args.length!=2 && args.length!=3)
			System.out.println("You should input 2 or 3 parameters");
		int id=0;
		int beginid=0;
		int endid=0;
		String path=args[0];
		if(args.length==2)
			id=Integer.parseInt(args[1]);
		if(args.length==3)
		{
			beginid=Integer.parseInt(args[1]);
			endid=Integer.parseInt(args[2]);
		}
		try {
			Class.forName("com.mysql.jdbc.Driver");
			ct=DriverManager.getConnection("jdbc:mysql://localhost:3306/leafdata","root","6356763");

			if(args.length==2)
				downloadByid(id,path);
			if(args.length==3){
				for(int i=beginid;i<=endid;i++)
				downloadByid(i,path);
			}
		}
		catch (Exception e) {
			// TODO: handle exception
		}
	}
	/*
	 * 根据leafList中的id去下载所有同名的图片
	 */
	public static void downloadByid(int id,String path){
		try {
			Statement stm=ct.createStatement();
			ResultSet res = stm.executeQuery("select * from leafC where id='"+id+"'");
			if(!res.next())
				System.out.println("Wrong input!");
			String leafName=res.getString("name");
			PreparedStatement ps=ct.prepareStatement("select * from leaf where name=?");
			ps.setString(1, leafName);
			ResultSet res2=ps.executeQuery();
			while(res2.next()){
				int lid=res2.getInt("id");	
				downloadPicByid(lid,leafName,path);
			}
		} catch (Exception e) {
			// TODO: handle exception
		}
		
	}
	
	public static void downloadPicByid(int lid,String leafName,String path)throws Exception{
		URL url=new URL("http://img.plantphoto.cn/image2/b/"+lid+".jpg");
		File outFile=new File(path+"/"+leafName+lid+".jpg");
		OutputStream os=new FileOutputStream(outFile);
		InputStream is=url.openStream();
		byte[] buff=new byte[1024];
		while(true){
			int readed=is.read(buff);
			if(readed==-1){
				break;
			}
			byte[] temp =new byte[readed];
			System.arraycopy(buff, 0, temp, 0, readed);
			os.write(temp);
		}
		is.close();
		os.close();
		System.out.println(lid+" done!");
	}
	public static void downloadPic(int begin,int end,String path) throws Exception{
		for(int i=begin;i<=end;i++){
			URL url=new URL("http://img.plantphoto.cn/image2/b/"+i+".jpg");
			File outFile=new File(path+"/"+i+".jpg");
			OutputStream os=new FileOutputStream(outFile);
			InputStream is=url.openStream();
			byte[] buff=new byte[1024];
			while(true){
				int readed=is.read(buff);
				if(readed==-1){
					break;
				}
				byte[] temp =new byte[readed];
				System.arraycopy(buff, 0, temp, 0, readed);
				os.write(temp);
			}
			is.close();
			os.close();
			System.out.println(i+" done!");
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
	
}
