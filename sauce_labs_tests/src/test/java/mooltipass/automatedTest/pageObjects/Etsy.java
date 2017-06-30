package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Etsy extends AbstractPage{
	
	public Etsy (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(id = "sign-in")
	private WebElement loginBtn;
	@FindBy(id = "username-existing")
	private WebElement email;

	@FindBy(id = "password-existing")
	private WebElement password;
	
	@FindBy(id = "signin-button")
	private WebElement submitLogin;
	
	
	@FindBy(xpath = "//a[@class='subnav-text-link sign-out']")
	private WebElement logoutBtn;
	
	@FindBy(xpath = "//span[@class='nav-icon nav-icon-image nav-icon-circle']")
	private WebElement dashBoard;
	
	public void enterEmail(String value){
		email.sendKeys(value);
	}

	public void enterPassword(String value){
		password.sendKeys(value);
	}
	
	public void goToLogin(){
		loginBtn.click();
	}
	
	public void submit(){
	submitLogin.click();
	}
	public void goTodDashboard()
	{
		waitUntilAppears(dashBoard);
		dashBoard.click();
		
	}
	public void logout(){
		logoutBtn.click();
	}
	
	public boolean checkLogin(){
		waitUntilAppears(By.xpath("//span[@class='nav-icon nav-icon-image nav-icon-circle']"));
		return isElementPresent(By.xpath("//span[@class='nav-icon nav-icon-image nav-icon-circle']"));
	}
	public boolean checkAtLoginPage(){
		return isElementPresent(By.id("username-existing"));
	}
}
