package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.interactions.Actions;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Kenwoodworld extends AbstractPage{

	public Kenwoodworld(WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	
	@FindBy(id = "ctl00_ContentPlaceHolderAll_ContentPlaceHolderHeaderArea_ctl00_RegisterOrSignin1_hyperlinkSignin")
	private WebElement loginBtn;

	@FindBy(xpath = "//input[@id='ctl00_ContentPlaceHolderAll_ContentPlaceHolderMain_Signin1_textboxEmailAddress']")
	private WebElement email;

	@FindBy(xpath = "//input[@id='ctl00_ContentPlaceHolderAll_ContentPlaceHolderMain_Signin1_textboxPassword']")
	private WebElement password;

	@FindBy(xpath = "//input[@id='ctl00_ContentPlaceHolderAll_ContentPlaceHolderMain_Signin1_buttonSignIn']")
	private WebElement submitLogin;
	
	@FindBy(id = "ctl00_ContentPlaceHolderAll_ContentPlaceHolderHeaderArea_ctl00_SignoutMyAccount1_hyperlinkSignOut")
	private WebElement logoutBtn;
	
	
	public void goToLogin(){
		waitUntilAppears(loginBtn);
		loginBtn.click();
	}
	
	public void enterEmail(String value){
		waitUntilAppears(email);
		email.sendKeys(value);
	}

	public void enterPassword(String value){
		password.sendKeys(value);
	}
	
	public void submit(){
		submitLogin.click();
		}
	
	public boolean checkLogin(){

		waitUntilAppears(By.id("ctl00_ContentPlaceHolderAll_ContentPlaceHolderHeaderArea_ctl00_SignoutMyAccount1_hyperlinkSignOut"));
		return isElementPresent(By.id("ctl00_ContentPlaceHolderAll_ContentPlaceHolderHeaderArea_ctl00_SignoutMyAccount1_hyperlinkSignOut"));
	}


	public boolean checkAtLoginPage(){
		return isElementPresent(By.xpath("//input[@id='ctl00_ContentPlaceHolderAll_ContentPlaceHolderMain_Signin1_textboxEmailAddress']"));
	}
	
	public void logout(){
		logoutBtn.click();

	}
}